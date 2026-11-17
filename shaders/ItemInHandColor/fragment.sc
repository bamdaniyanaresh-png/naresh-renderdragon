$input v_vcol, v_lit, v_atten

#include <bgfx_shader.sh>
#include <MinecraftRenderer.Materials/ActorUtil.dragonh>
#include <naresh.sh>

uniform vec4 ChangeColor;
uniform vec4 OverlayColor;
uniform vec4 ColorBased;
uniform vec4 MultiplicativeTintColor;

void main() {
  #if defined(DEPTH_ONLY) || defined(INSTANCING)
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    return;
  #endif

  vec4 col = vec4(mix(vec3(1.0, 1.0, 1.0), v_vcol.rgb, ColorBased.x), 1.0);

  #ifdef MULTI_COLOR_TINT
    col = applyMultiColorChange(col, ChangeColor.rgb, MultiplicativeTintColor.rgb);
  #else
    col = applyColorChange(col, ChangeColor, col.a);
    col.a *= ChangeColor.a;
  #endif

  col = applyOverlayColor(col, OverlayColor);

  #ifdef ALPHA_TEST
    if (col.a < 0.5) {
      discard;
    }
  #endif

  col.rgb = nrsToLinear(col.rgb)*v_lit.rgb;
  col.rgb = nrsEntityFinish(col.rgb, v_atten.rgb, v_atten.a);
  gl_FragColor = col;
}
