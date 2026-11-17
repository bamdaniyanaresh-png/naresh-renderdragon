#ifndef NRS_TERRAIN_H
#define NRS_TERRAIN_H

// Lighting of the world (RenderChunk): sun and moon, sky light, block shadows,
// torch light, fog, water, glass and shiny blocks.

#include "sky.h"
#include "rain.h"
#include "torch.h"
#include "tone.h"

// ---- light curves ----

// sky light level (light map y, level/16) -> 0 to 1
float nrsSkyCurve(float uvy) {
  return pow(saturate(uvy*(16.0/15.0)), 2.3);
}

// block shadow: places that see the sky get the sun, the edge is soft
float nrsShadowMask(float uvy) {
  float w = NRS_SHADOW_SOFTNESS/16.0;
  return smoothstep(0.875 - w, 0.875, uvy);
}

// vertex colour -> rgb = biome tint, a = shade of the face and corner
vec4 nrsVertexTint(vec4 c) {
  if (abs(c.r - c.g) < 0.000002 && abs(c.g - c.b) < 0.000002) {
    return vec4(1.0, 1.0, 1.0, c.r);
  }
  // a tinted vertex carries its shade in the colour
  return vec4(c.rgb, 1.0);
}

// ---- fog ----

// the fog comes closer while it rains
float nrsFogAmount(highp float dist, float rain) {
  float x = dist/(NRS_FOG_RANGE*(1.0 - 0.35*rain));
  float f = 1.0 - exp(-2.5*x*x*x);
  return saturate(f + 0.03*(1.0 - exp(-dist*(1.0/80.0))));
}

// ---- vertex part ----
// ambient  sky light colour
// sunCol   colour of the direct light (sun or moon)
// fogCol   colour of the air, fogAmt how much of it is in front of the vertex
// rain     0 to 1
void nrsTerrainVertex(
  float tod, float rain, highp vec3 wpos, vec2 uv1,
  out vec3 ambient, out vec3 sunCol, out vec3 fogCol, out float fogAmt
) {
  highp vec3 S = nrsSunTrue(tod);
  highp vec3 L;
  vec3 Lcol;
  nrsLightFromSun(S, rain, L, Lcol);

  sunCol = Lcol*NRS_SUN_TINT;
  ambient = nrsAmbientColor(S.y, rain)*NRS_AMBIENT_TINT*NRS_AMBIENT_POWER;

  highp float dist = length(wpos);
  highp vec3 dir = wpos/max(dist, 0.001);
  fogCol = nrsHorizonColor(dir, S, rain)*(0.04 + 0.96*nrsSkyCurve(uv1.y));
  fogAmt = nrsFogAmount(dist, rain);
}

// ---- surface lighting ----

// diffuse light on a surface: sun, sky, block light
// wrap lets light bend around leaves and plants (0 for solid blocks)
vec3 nrsLightSurface(
  vec3 albedo, highp vec3 N, highp vec3 L, vec3 sunCol, vec3 ambient, vec2 uv1, float ao, float wrap
) {
  float skyL = nrsSkyCurve(uv1.y);
  float shadow = nrsShadowMask(uv1.y);
  float NoL = saturate((dot(N, L) + wrap)/(1.0 + wrap));

  vec3 sun = sunCol*(NoL*shadow);

  // sky light: more from above, a warm bounce from below
  float hemi = N.y*0.5 + 0.5;
  vec3 ground = ambient*vec3(0.42, 0.40, 0.38) + sunCol*(0.04*shadow);
  vec3 sky = mix(ground, ambient, hemi)*skyL;
  sky *= mix(NRS_SHADOW_TINT, vec3_splat(1.0), shadow);
  sky *= pow(ao, NRS_AO_POWER);

  vec3 torch = nrsTorchLight(uv1);
  vec3 starlight = vec3(0.012, 0.014, 0.020);

  return albedo*(sun + sky + torch + starlight);
}

float nrsFresnel(float f0, float NoV) {
  return f0 + (1.0 - f0)*pow(1.0 - NoV, 5.0);
}

// shiny spot of the sun or the moon
vec3 nrsSunSpec(highp vec3 N, highp vec3 V, highp vec3 L, vec3 sunCol, float rough, vec3 F) {
  highp vec3 H = normalize(V + L);
  float NoH = saturate(dot(N, H));
  float NoL = saturate(dot(N, L));
  float shin = 2.0/(rough*rough) - 2.0;
  float nrm = (shin + 8.0)*(1.0/(8.0*NRS_PI));
  return F*sunCol*(pow(NoH, shin)*nrm*NoL);
}

// sky, sun and clouds seen in a surface
highp vec3 nrsReflectSky(highp vec3 V, highp vec3 Nrm, highp vec3 S, highp float time, float rain) {
  highp vec3 R = reflect(-V, Nrm);
  // a smooth mirror of the part below the horizon (abs() would leave a crease)
  R.y = sqrt(R.y*R.y + 0.0025);
  R = normalize(R);
  return nrsSkyFull(R, S, time, 0.5, rain);
}

// wavy water: two layers of noise that move, the slope gives the normal
highp float nrsWaveHeight(highp vec2 p, highp float t) {
  highp float ts = t*NRS_WATER_WAVE_SPEED;
  return nrsNoise(p*1.7 + vec2(ts*0.35, ts*0.21))*0.55
       + nrsNoise(p*3.9 - vec2(ts*0.50, ts*0.30))*0.35
       + nrsNoise(p*8.3 + vec2(ts*0.80, -ts*0.60))*0.10;
}

highp vec3 nrsWaterNormal(highp vec3 N, highp vec2 gxz, highp float t, float strength) {
  if (N.y < 0.5) {
    return N;
  }
  const float e = 0.08;
  highp float h0 = nrsWaveHeight(gxz, t);
  highp float hx = nrsWaveHeight(gxz + vec2(e, 0.0), t);
  highp float hz = nrsWaveHeight(gxz + vec2(0.0, e), t);
  highp float k = 0.03/e*strength*NRS_WATER_WAVE_STRENGTH;
  return normalize(vec3((h0 - hx)*k, 1.0, (h0 - hz)*k));
}

// ---- material flags in the alpha of the block texture (255 = plain block) ----

float nrsBlockCode(float a) {
  float c = floor(a*255.0 + 0.1);
  return c < 0.5 ? 255.0 : c;
}

bool nrsBitSet(float code, float bit) {
  return mod(floor(code/bit), 2.0) > 0.5;
}

vec3 nrsMetalF0(float code) {
  float idx = mod(floor(code/4.0), 4.0);
  if (idx < 0.5) return vec3(1.000, 0.782, 0.344);   // gold
  if (idx < 1.5) return vec3(0.972, 0.960, 0.915);   // silver
  if (idx < 2.5) return vec3(0.955, 0.638, 0.538);   // copper
  return vec3(0.562, 0.565, 0.578);                  // iron
}

// ---- fragment part ----
// tex      texture colour, the alpha test is done by the caller
// vcol     vertex colour
// tint     from nrsVertexTint
// uv1      light map from nrsLightUv
// wpos     camera relative position, gpos the global one
// N        normal that faces the camera
// dpx, dpy, dux, duy  screen space derivatives of wpos and uv
vec4 nrsTerrainShade(
  vec4 tex, vec4 vcol, vec4 tint, vec2 uv1,
  highp vec3 wpos, highp vec3 gpos, highp vec3 N,
  highp vec3 dpx, highp vec3 dpy, vec2 dux, vec2 duy,
  vec3 ambient, vec3 sunCol, vec3 fogCol, float fogAmt,
  bool isWater, float tod, float rain, highp float time
) {
  vec4 outc = vec4(0.18, 0.18, 0.18, 1.0);

  highp vec3 S = nrsSunTrue(tod);
  highp vec3 L;
  vec3 Lcol;
  nrsLightFromSun(S, rain, L, Lcol);

  highp vec3 V = normalize(-wpos);
  vec3 albedo = nrsToLinear(tex.rgb*tint.rgb);
  float ao = tint.a;
  float shadow = nrsShadowMask(uv1.y);
  float skyL = nrsSkyCurve(uv1.y);
  vec3 lit = vec3_splat(0.0);

  #if defined(SEASONS)

    // season variants are shaded like leaves
    lit = nrsLightSurface(albedo, N, L, sunCol, ambient, uv1, ao, 0.4);

  #elif defined(TRANSPARENT)

    vec3 torch = nrsTorchLight(uv1);

    if (isWater) {
      highp vec3 Nw = N;
      #if NRS_WATER_TYPE == 2
        // far water is flatter, so it does not shimmer
        float calm = smoothstep(70.0, 220.0, length(wpos));
        Nw = nrsWaterNormal(N, gpos.xz, time, 1.0 - calm);
      #endif

      // rain drop rings on the water
      #if NRS_RAIN && NRS_RAIN_RIPPLE
        if (rain > 0.001) {
          vec3 rn = nrsRainRipple(gpos.xz, time);
          Nw = normalize(Nw + vec3(rn.x, 0.0, rn.y)*(rain*NRS_RAIN_RIPPLE_STRENGTH*0.6));
        }
      #endif

      float NoV = saturate(dot(Nw, V));
      float NoL = saturate(dot(Nw, L));

      // the body of the water: the colour of the biome water, lit by sun, sky and torch
      vec3 wcol = nrsToLinear(vcol.rgb*0.8);
      vec3 scatter = wcol*(sunCol*(0.5*NoL*shadow) + ambient*(0.8*skyL) + torch + vec3(0.012, 0.014, 0.020));

      #ifdef NRS_NO_REFLECTION
        // no reflection, the water keeps the opacity of the vertex colour
        lit = scatter;
        outc.a = vcol.a;
      #else
        float F = nrsFresnel(0.02, NoV);
        vec3 refl = nrsReflectSky(V, Nw, S, time, rain)*skyL;
        vec3 spec = nrsSunSpec(Nw, V, L, sunCol*shadow, 0.06, vec3_splat(0.5));
        float a = saturate(NRS_WATER_CLARITY + F + nrsLuma(spec));
        lit = (scatter*NRS_WATER_CLARITY + refl*F + spec)/a;
        outc.a = a;
      #endif
      outc.a = mix(outc.a, 1.0, fogAmt);
    } else {
      // glass, ice and other transparent blocks
      float NoV = saturate(dot(N, V));
      vec3 base = nrsLightSurface(albedo, N, L, sunCol, ambient, uv1, ao, 0.0);
      #ifdef NRS_NO_REFLECTION
        lit = base;
        outc.a = tex.a;
      #else
        float F = nrsFresnel(0.04, NoV);
        vec3 refl = nrsReflectSky(V, N, S, time, rain)*skyL;
        vec3 spec = nrsSunSpec(N, V, L, sunCol*shadow, 0.08, vec3_splat(0.5));
        float a = saturate(tex.a + F + nrsLuma(spec));
        lit = (base*tex.a + refl*F + spec)/a;
        outc.a = a;
      #endif
    }

  #else

    #if defined(ALPHA_TEST)
      if (vcol.b == 0.0) {
        // redstone wire is not lit
        outc.rgb = tex.rgb*vcol.rgb;
        return outc;
      }
    #endif

    if (vcol.a < 0.001) {
      // leaves
      lit = nrsLightSurface(albedo, N, L, sunCol, ambient, uv1, ao, 0.35);
    } else {
      #if defined(ALPHA_TEST)
        // tall grass, flowers, crops
        lit = nrsLightSurface(albedo, N, L, sunCol, ambient, uv1, ao, 0.5);
      #else
        // opaque blocks, the flags are in the texture alpha
        float code = nrsBlockCode(tex.a);
        bool isMetal = !nrsBitSet(code, 128.0);
        bool isSmooth = !nrsBitSet(code, 64.0);
        bool isGlow = !nrsBitSet(code, 32.0);

        float NoV = saturate(dot(N, V));

        if (isMetal) {
          vec3 f0 = mix(albedo, nrsMetalF0(code), 0.8);
          vec3 diffuse = nrsLightSurface(albedo, N, L, sunCol, ambient, uv1, ao, 0.0);
          vec3 spec = nrsSunSpec(N, V, L, sunCol*shadow, 0.12, f0);
          vec3 refl = ambient*skyL*f0;
          #ifndef NRS_NO_REFLECTION
            if (isSmooth) {
              refl = nrsReflectSky(V, N, S, time, rain)*skyL*f0;
            }
          #endif
          lit = diffuse*0.25 + spec + refl*nrsFresnel(0.9, NoV);
        } else {
          lit = nrsLightSurface(albedo, N, L, sunCol, ambient, uv1, ao, 0.0);
          #ifndef NRS_NO_REFLECTION
            if (isSmooth) {
              float F = nrsFresnel(0.04, NoV);
              lit += nrsReflectSky(V, N, S, time, rain)*(skyL*F);
              lit += nrsSunSpec(N, V, L, sunCol*shadow, 0.08, vec3_splat(0.04));
            }
          #endif

          // rain: the ground gets wet, darker, and puddles reflect the sky
          #if NRS_RAIN
            if (rain > 0.001 && N.y > 0.9) {
              float expo = saturate((uv1.y - 0.7)*5.0);
              float closeness = saturate(1.0 - fogAmt*2.2);
              float wet = rain*expo*closeness*NRS_RAIN_WET;
              if (wet > 0.002) {
                float puddle = nrsWetMask(gpos.xz);
                lit *= 1.0 - 0.30*wet;
                highp vec3 Np = N;
                vec4 rip = vec4_splat(0.0);
                #if NRS_RAIN_RIPPLE
                  vec3 rn = nrsRainRipple(gpos.xz, time);
                  float k = wet*puddle*NRS_RAIN_RIPPLE_STRENGTH;
                  Np = normalize(N + vec3(rn.x, 0.0, rn.y)*(k*0.7));
                  float bright = max(saturate(nrsLuma(ambient)*1.6), pow(uv1.x, 1.4)*0.8);
                  rip = nrsRainRippleShade(rn, k, bright);
                #endif
                float Fw = nrsFresnel(mix(0.04, 0.22, puddle), saturate(dot(Np, V)));
                lit += nrsReflectSky(V, Np, S, time, rain)*(Fw*wet*mix(0.25, 1.0, puddle));
                lit *= 1.0 - rip.a;
                lit += rip.rgb;
              }
            }
          #endif
        }
        if (isGlow) {
          lit += albedo*0.5;
        }
      #endif
    }

  #endif

  // air between the camera and the surface
  outc.rgb = nrsTone(mix(lit, fogCol, fogAmt));
  return outc;
}

// ---- redstone helpers ----

vec3 nrsChunkBound(vec3 oc, vec3 cp, vec3 N, vec3 bp) {
  vec3 cc = vec3_splat(0.0);
  bool cbx = abs(8.0 - cp.x) > (8.0 - 1.5/16.0);
  bool cby = abs(8.0 - cp.y) > (8.0 - 1.5/16.0);
  bool cbz = abs(8.0 - cp.z) > (8.0 - 1.5/16.0);
  bool bbx = abs(0.5 - bp.x) > (0.5 - 0.5/16.0);
  bool bby = abs(0.5 - bp.y) > (0.5 - 0.5/16.0);
  bool bbz = abs(0.5 - bp.z) > (0.5 - 0.5/16.0);
  if ((cbx || cbz) && ((bbx && bby) || (bby && bbz) || (bbz && bbx))) {
    bool flag = !(N.y > 0.9) && ((cbx && cby) || (cby && cbz) || (cbz && cbx));
    vec4 col = vec4(1.0, 1.0, 1.0, 0.2);                           // other edges
    if (flag && cbx && cbz) col = vec4(0.0, 1.0, 0.0, 0.5);       // y axis
    else if (flag && !cbx && cbz) col = vec4(1.0, 0.0, 0.0, 0.5); // x axis
    else if (flag && cbx && !cbz) col = vec4(0.0, 0.0, 1.0, 0.5); // z axis
    cc = mix(cc, col.rgb, col.a);
  }
  return oc + cc;
}

vec3 nrsLightBound(vec3 outcol, vec2 uv1, vec3 N, vec3 blopos) {
  bool lba = (uv1.x > 0.4375 && uv1.x < 0.498) && (N.y > 0.9) && (abs(blopos.x - 0.5) > 0.46875 || abs(blopos.z - 0.5) > 0.46875);
  return lba ? mix(outcol, vec3(1.0, 0.5, 0.0), 0.5) : outcol;
}

#endif
