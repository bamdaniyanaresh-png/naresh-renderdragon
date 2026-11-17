#!/usr/bin/env python3
"""Build tool for Naresh RenderDragon.

  python naresh.py setup --shaderc PATH --vanilla DIR   get the compiler and the game materials ready
  python naresh.py check                                look for mistakes in the project files
  python naresh.py build [-p android]                   compile the materials only
  python naresh.py pack  [-p android]                   compile everything and make the .mcpack

It needs Python 3.11 or newer and the lazurite package (pip install -r requirements.txt).
"""
import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

try:
    import tomllib
except ImportError:  # Python older than 3.11
    tomllib = None

ROOT = Path(__file__).resolve().parent
PROJECT = ROOT / "shaders"
VANILLA = ROOT / "vanilla"
TOOLS = ROOT / "tools"
BUILD = ROOT / "build"
PLATFORMS = ("android", "ios", "windows", "all")


def fail(text):
    print("error: " + text)
    sys.exit(1)


def load_pack():
    if tomllib is None:
        fail("Python 3.11 or newer is needed to read pack.toml")
    with open(ROOT / "pack.toml", "rb") as f:
        return tomllib.load(f)


def find_shaderc(given):
    names = ["shaderc.exe", "shaderc"]
    candidates = [Path(given)] if given else [TOOLS / n for n in names]
    for c in candidates:
        if c.is_file():
            return c
    found = shutil.which("shaderc")
    if found:
        return Path(found)
    fail("shaderc was not found. Run setup with --shaderc PATH or copy it to tools/")


def compiler_env():
    """Environment for lazurite. On Termux shaderc needs libc++_shared.so, so it is
    copied next to the tools and that folder is added to LD_LIBRARY_PATH."""
    env = os.environ.copy()
    prefix = Path(os.environ.get("PREFIX", "/data/data/com.termux/files/usr"))
    lib = prefix / "lib" / "libc++_shared.so"
    if lib.is_file():
        TOOLS.mkdir(exist_ok=True)
        local = TOOLS / "libc++_shared.so"
        if not local.is_file():
            shutil.copy2(lib, local)
        old = env.get("LD_LIBRARY_PATH")
        env["LD_LIBRARY_PATH"] = str(TOOLS) + (":" + old if old else "")
    return env


def run_lazurite(args):
    cmd = [sys.executable, "-m", "lazurite"] + args
    print("$ " + " ".join(str(a) for a in cmd))
    result = subprocess.run(cmd, cwd=ROOT, env=compiler_env())
    if result.returncode != 0:
        fail("lazurite stopped with code %d" % result.returncode)


def material_names():
    return sorted(p.name for p in PROJECT.iterdir() if p.is_dir())


# setup

def cmd_setup(args):
    if args.shaderc:
        src = Path(args.shaderc)
        if not src.is_file():
            fail("%s is not a file" % src)
        TOOLS.mkdir(exist_ok=True)
        dst = TOOLS / src.name
        shutil.copy2(src, dst)
        dst.chmod(dst.stat().st_mode | 0o111)
        print("shaderc copied to %s" % dst)

    if args.vanilla:
        src = Path(args.vanilla)
        if not src.is_dir():
            fail("%s is not a folder" % src)
        VANILLA.mkdir(exist_ok=True)
        bins = sorted(src.glob("*.material.bin"))
        jsons = sorted(src.glob("*.material.json"))
        if bins:
            # compact json copies of the game materials, used as the merge source
            run_lazurite(["serialize"] + [str(b) for b in bins] + ["-o", str(VANILLA)])
        for j in jsons:
            shutil.copy2(j, VANILLA / j.name)
        if not bins and not jsons:
            fail("no *.material.bin or *.material.json files in %s" % src)
        print("%d materials are in vanilla/" % len(list(VANILLA.glob("*.material.json"))))

    if not args.shaderc and not args.vanilla:
        print("nothing to do, use --shaderc and/or --vanilla")


# check

def parse_varying(path):
    names = set()
    for line in path.read_text(encoding="utf-8").splitlines():
        m = re.match(r"\s*(?:centroid\s+)?\w+\s+(\w+)\s*:\s*\w+", line)
        if m:
            names.add(m.group(1))
    return names


def cmd_check(args):
    problems = []
    pack = load_pack()
    names = material_names()
    for name in names:
        d = PROJECT / name
        for f in ("vertex.sc", "fragment.sc", "varying.def.sc", "config.json"):
            if not (d / f).is_file():
                problems.append("%s: %s is missing" % (name, f))
        for f in ("config.json", "material.json"):
            p = d / f
            if p.is_file():
                try:
                    json.loads(p.read_text(encoding="utf-8"))
                except ValueError as e:
                    problems.append("%s/%s is not valid json: %s" % (name, f, e))
        if (d / "varying.def.sc").is_file():
            known = parse_varying(d / "varying.def.sc")
            for stage in ("vertex.sc", "fragment.sc"):
                if not (d / stage).is_file():
                    continue
                for line in (d / stage).read_text(encoding="utf-8").splitlines():
                    s = line.strip()
                    if s.startswith("$input") or s.startswith("$output"):
                        for var in s.split(None, 1)[1].split(","):
                            var = var.strip()
                            if var and var not in known:
                                problems.append("%s/%s: %s is not in varying.def.sc" % (name, stage, var))
    project = json.loads((PROJECT / "project.json").read_text(encoding="utf-8"))
    listed = set(project["base_profile"]["include_patterns"])
    if listed != set(names):
        problems.append("project.json include_patterns does not match the material folders")
    for sp in pack.get("subpack", []):
        for m in sp["materials"]:
            if m not in names:
                problems.append("subpack %s uses unknown material %s" % (sp["name"], m))
    if problems:
        for p in problems:
            print("problem: " + p)
        sys.exit(1)
    print("%d materials, %d subpacks, no problems found" % (len(names), len(pack.get("subpack", []))))
    if not any(VANILLA.glob("*.material.json")):
        print("note: vanilla/ is empty, run setup --vanilla before building")


# build

def compile_materials(platform, out_dir, shaderc, define=None, only=None):
    """Compile into out_dir. Returns the list of material names that were written."""
    out_dir.mkdir(parents=True, exist_ok=True)
    # lazurite runs in the repo folder, so the paths here are relative to it
    project = PROJECT.relative_to(ROOT).as_posix()
    cmd = ["build", project, "--shaderc", str(shaderc), "-p", platform, "-o", str(out_dir)]
    if define:
        cmd += ["-d", define]
    if only:
        cmd += ["-m"] + ["%s/%s" % (project, m) for m in only]
    run_lazurite(cmd)
    wanted = only or material_names()
    written = [m for m in wanted if (out_dir / (m + ".material.bin")).is_file()]
    if only and len(written) != len(only):
        # the material filter did not match, compile all and keep what is needed
        print("material filter gave no result, compiling all materials instead")
        everything = out_dir.parent / (out_dir.name + "_all")
        compile_materials(platform, everything, shaderc, define, None)
        for m in only:
            src = everything / (m + ".material.bin")
            if not src.is_file():
                fail("%s was not compiled" % m)
            shutil.copy2(src, out_dir / src.name)
        shutil.rmtree(everything)
        written = list(only)
    missing = [m for m in wanted if m not in written]
    if missing:
        fail("not compiled: " + ", ".join(missing))
    return written


def need_vanilla():
    if not any(VANILLA.glob("*.material.json")):
        fail("vanilla/ is empty. Run: python naresh.py setup --vanilla <folder with the game material.bin files>")


def cmd_build(args):
    need_vanilla()
    shaderc = find_shaderc(args.shaderc)
    out = BUILD / "materials" / args.profile
    if out.exists():
        shutil.rmtree(out)
    compile_materials(args.profile, out, shaderc, args.define)
    print("materials are in %s" % out)


# pack

def make_manifest(pack, subpacks):
    info = pack["pack"]
    return {
        "format_version": 2,
        "header": {
            "name": info["name"],
            "description": info["description"],
            "uuid": info["uuid"],
            "version": info["version"],
            "min_engine_version": info["min_engine_version"],
        },
        "modules": [{
            "type": "resources",
            "uuid": info["module_uuid"],
            "version": info["version"],
        }],
        "subpacks": [
            {"folder_name": s["folder"], "name": s["name"], "memory_tier": 1}
            for s in subpacks
        ],
        "metadata": {"authors": info["authors"], "url": info.get("url", "")},
    }


def cmd_pack(args):
    need_vanilla()
    pack = load_pack()
    subpacks = pack.get("subpack", [])
    shaderc = find_shaderc(args.shaderc)
    info = pack["pack"]

    folder = BUILD / "pack"
    if folder.exists():
        shutil.rmtree(folder)
    folder.mkdir(parents=True)

    # assets and notices
    for item in (ROOT / "assets").iterdir():
        if item.is_dir():
            shutil.copytree(item, folder / item.name)
        else:
            shutil.copy2(item, folder / item.name)
    shutil.copy2(ROOT / "NOTICE.md", folder / "NOTICE.txt")

    # main pack
    print("main pack")
    compile_materials(args.profile, folder / "renderer" / "materials", shaderc)

    # subpacks
    for sp in subpacks:
        print("subpack " + sp["name"])
        target = folder / "subpacks" / sp["folder"]
        target.mkdir(parents=True, exist_ok=True)
        if sp["materials"]:
            compile_materials(args.profile, target / "renderer" / "materials", shaderc, sp["define"], sp["materials"])

    (folder / "manifest.json").write_text(json.dumps(make_manifest(pack, subpacks), indent=2), encoding="utf-8")

    version = ".".join(str(v) for v in info["version"])
    out_name = "%s-%s-%s.mcpack" % (info["name"].replace(" ", "-"), version, args.profile)
    out_path = Path(args.output) if args.output else BUILD / out_name
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(out_path, "w", zipfile.ZIP_DEFLATED) as z:
        for p in sorted(folder.rglob("*")):
            name = p.relative_to(folder).as_posix()
            if p.is_dir():
                z.write(p, name + "/")  # keeps empty folders such as subpacks/medium
            else:
                z.write(p, name)
    print("pack written: %s" % out_path)


def main():
    parser = argparse.ArgumentParser(description="Naresh RenderDragon build tool")
    sub = parser.add_subparsers(dest="command", required=True)

    p = sub.add_parser("setup", help="copy shaderc and the game materials into the project")
    p.add_argument("--shaderc", help="path of the shaderc compiler")
    p.add_argument("--vanilla", help="folder with the game *.material.bin (or *.material.json) files")
    p.set_defaults(func=cmd_setup)

    p = sub.add_parser("check", help="look for mistakes in the project files")
    p.set_defaults(func=cmd_check)

    p = sub.add_parser("build", help="compile the materials")
    p.add_argument("-p", "--profile", default="android", choices=PLATFORMS)
    p.add_argument("-d", "--define", help="extra macro, for example QUALITY_LOW")
    p.add_argument("--shaderc", help="path of the shaderc compiler")
    p.set_defaults(func=cmd_build)

    p = sub.add_parser("pack", help="make the .mcpack with all subpacks")
    p.add_argument("-p", "--profile", default="android", choices=PLATFORMS)
    p.add_argument("-o", "--output", help="output file")
    p.add_argument("--shaderc", help="path of the shaderc compiler")
    p.set_defaults(func=cmd_pack)

    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
