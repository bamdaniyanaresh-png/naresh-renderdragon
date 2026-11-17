#ifndef NRS_SKY_H
#define NRS_SKY_H

// The sky: colour gradient, sun, big moon with a glow, northern lights and clouds.
// Every function takes a view direction d and the direction of the sun S (nrsSunTrue).

#include "daylight.h"
#include "stars.h"

// colour gradient from the zenith to the horizon
highp vec3 nrsSkyBase(highp vec3 d, highp vec3 S, float rain) {
  float sy = S.y;
  float day = nrsDayAmt(sy);
  float dusk = nrsDuskAmt(sy);

  vec3 zen = mix(vec3(0.003, 0.006, 0.022), vec3(0.16, 0.36, 0.85), day);
  vec3 hor = mix(vec3(0.012, 0.022, 0.055), vec3(0.62, 0.78, 1.0)*1.05, day);
  zen = mix(zen, vec3(0.07, 0.10, 0.30), dusk*0.85);
  hor = mix(hor, vec3(1.25, 0.50, 0.20), dusk*0.85);

  float h = saturate(d.y);
  highp vec3 col = mix(zen, hor, pow(1.0 - h, 3.2));

  // the side of the sun is warmer at sunrise and sunset
  highp vec2 dxz = d.xz/max(length(d.xz), 0.0001);
  highp vec2 sxz = S.xz/max(length(S.xz), 0.0001);
  float side = nrsSq(saturate(dot(dxz, sxz)*0.5 + 0.5));
  float f = dusk*(0.25 + 0.75*side)*pow(1.0 - h, 1.2);
  col = mix(col, vec3(1.3, 0.45, 0.28), f*0.55);

  // rain: a grey sky, darker when the sun is down
  vec3 grey = mix(vec3(0.26, 0.29, 0.34), vec3(0.50, 0.53, 0.57), pow(1.0 - h, 2.0))*(0.10 + 0.90*day);
  col = mix(col, grey, rain*0.85);

  // below the horizon the sky gets darker
  col = mix(col, col*0.55, saturate(-d.y*3.0));
  return col;
}

// soft light around the sun
highp vec3 nrsSunGlow(highp vec3 d, highp vec3 S) {
  float sd = max(dot(d, S), 0.0);
  float up = smoothstep(-0.10, 0.04, S.y);
  float g = pow(sd, 6.0)*0.20 + pow(sd, 48.0)*0.7;
  return vec3(1.0, 0.82, 0.55)*(g*up);
}

highp vec3 nrsSunLayer(highp vec3 d, highp vec3 S, float rain) {
  float sd = max(dot(d, S), 0.0);
  float up = smoothstep(-0.10, 0.04, S.y);
  float cr = cos(NRS_SUN_SIZE);
  float disc = smoothstep(cr - 0.0010, cr + 0.0003, sd);
  return (nrsSunGlow(d, S) + vec3(1.0, 0.93, 0.8)*(disc*14.0*up))*(1.0 - rain);
}

// the moon is opposite to the sun: big disc with dark patches and a glow
highp vec3 nrsMoonLayer(highp vec3 d, highp vec3 S, float q, float rain) {
  highp vec3 M = -S;
  float up = smoothstep(-0.12, 0.10, M.y)*(1.0 - rain);
  if (up <= 0.0) {
    return vec3_splat(0.0);
  }
  float dayfade = 1.0 - 0.85*nrsDayAmt(S.y);
  highp float md = dot(d, M);
  float ang = 1.0 - md;

  float g = exp(-ang/0.0045)*0.9 + exp(-ang/0.03)*0.35 + exp(-ang/0.18)*0.10;
  highp vec3 col = vec3(0.55, 0.68, 1.0)*(g*NRS_MOON_GLOW);

  float r = NRS_MOON_SIZE;
  if (md > cos(r*1.2)) {
    highp vec3 T = abs(M.y) > 0.999 ? vec3(1.0, 0.0, 0.0) : normalize(cross(vec3(0.0, 1.0, 0.0), M));
    highp vec3 B = cross(M, T);
    highp float u = dot(d, T)/r;
    highp float v = dot(d, B)/r;
    float rr = sqrt(u*u + v*v);
    float inside = 1.0 - smoothstep(0.95, 1.0, rr);
    float mar = nrsFbm(vec2(u, v)*2.2 + vec2(3.0, 1.0), q > 0.75 ? 3.0 : 2.0);
    float dark = smoothstep(0.42, 0.62, mar)*0.38;
    float cr = nrsFbm(vec2(u, v)*7.0, 2.0);
    dark += smoothstep(0.55, 0.70, cr)*0.10;
    float nz = sqrt(max(1.0 - rr*rr, 0.0));
    float shade = (0.78 + 0.22*nz)*(1.0 - dark);
    col += vec3(0.95, 0.97, 1.05)*(NRS_MOON_POWER_DISC*shade*inside);
  }
  return col*(up*dayfade);
}

// northern lights, a few layers of curtains in the north
highp vec3 nrsAurora(highp vec3 d, highp vec3 S, highp float time, float rain) {
  float night = (1.0 - smoothstep(-0.30, -0.08, S.y))*(1.0 - rain);
  float vis = smoothstep(0.03, 0.25, d.y);
  if (night*vis < 0.003) {
    return vec3_splat(0.0);
  }
  highp float dy = max(d.y, 0.03);
  highp vec3 acc = vec3_splat(0.0);
  for (float i = 0.0; i < NRS_AURORA_STEPS; i++) {
    float t = i/(NRS_AURORA_STEPS - 1.0);
    float h = 0.5 + t*0.77;
    highp float px = d.x/dy*h;
    highp float pz = -d.z/dy*h;
    highp float wob = nrsNoise(vec2(px*0.22 + time*0.03, pz*0.05 + i*3.7))*2.4
                    + nrsNoise(vec2(px*0.7 - time*0.05, i*1.3 + 9.0))*0.7;
    float band = exp(-nrsSq(pz*0.22 - 1.2 + (wob - 1.5)*0.8)*5.0);
    float rays = 0.55 + 0.45*nrsNoise(vec2(px*2.2 + i*5.3, time*0.1));
    vec3 c = mix(vec3(0.10, 1.0, 0.45), vec3(0.55, 0.25, 0.95), smoothstep(0.45, 1.0, t));
    acc += c*(band*rays*(1.0 - t)*(0.17*8.0/NRS_AURORA_STEPS));
  }
  return acc*(night*vis*NRS_AURORA_POWER);
}

// the night sky: a purple band of dust and gas across the sky with stars in it.
// The colours are written as display colours and turned to linear light at the end.
highp vec3 nrsNightSky(highp vec3 d, highp vec3 S, highp float time, float rain) {
  float night = (1.0 - smoothstep(-0.30, -0.08, S.y))*(1.0 - rain);
  float vis = smoothstep(0.0, 0.18, d.y);
  if (night*vis < 0.003) {
    return vec3(0.0, 0.0, 0.0);
  }

  // the band follows a circle around the sky. across: distance from the circle, along: position on it
  const highp vec3 bandN = vec3(0.6027, 0.1507, 0.7836);
  const highp vec3 bandA = vec3(-0.6324, -0.5086, 0.5843);
  highp float across = dot(d, bandN) + time*0.0005;
  highp float along = dot(d, bandA);

  // wider band: the distance from the circle is divided by the width
  float x = abs(across)*(1.0/NRS_NIGHT_BAND_WIDTH);
  float core = exp(-x*x*18.0);
  float glow = exp(-x*x*4.0);

  vec3 col = vec3(0.015, 0.015, 0.035);

  // the gas and dust are only worked out inside the band, the rest of the sky is plain
  if (core > 0.004) {
    highp vec2 uv = vec2(across*2.2, along*1.3);
    float warp = nrsFbm(uv*0.7 + vec2(time*0.002, 0.0), 3.0);
    highp vec2 wuv = uv + vec2_splat(warp - 0.5)*0.16;

    float neb = nrsFbm(wuv*1.8, 3.0);
    float dust = nrsFbm(wuv*2.0 + vec2(3.7, 5.1), 3.0);
    float rift = smoothstep(0.35, 0.78, dust);
    float density = saturate(neb*core*(1.0 - rift*0.38));

    vec3 nebCol = mix(vec3(0.25, 0.28, 0.42), vec3(0.70, 0.48, 0.50), smoothstep(0.10, 0.50, density));
    nebCol = mix(nebCol, vec3(0.96, 0.82, 0.68), smoothstep(0.45, 0.85, density));
    col += nebCol*(density*1.5);
  }
  col += vec3(0.06, 0.05, 0.10)*glow;

  #if NRS_STARS
    col += nrsStars(d, time, core);
  #endif

  return nrsToLinear(col)*(night*vis*NRS_NIGHT_SKY_POWER);
}

// colour of the air at the horizon, used for fog
highp vec3 nrsHorizonColor(highp vec3 dir, highp vec3 S, float rain) {
  highp vec3 d = normalize(vec3(dir.x, 0.03, dir.z));
  return nrsSkyBase(d, S, rain) + nrsSunGlow(d, S)*(1.0 - rain);
}

#include "clouds.h"

// the whole sky along one ray. q is 1 for the sky itself and 0.5 for reflections.
// rain is 0 to 1: the sun and the moon fade away and the clouds close up.
highp vec3 nrsSkyFull(highp vec3 d, highp vec3 S, highp float time, float q, float rain) {
  highp vec3 L;
  vec3 Lcol;
  nrsLightFromSun(S, rain, L, Lcol);

  highp vec3 col = nrsSkyBase(d, S, rain);
  col += nrsSunLayer(d, S, rain);
  col += nrsMoonLayer(d, S, q, rain);

  #if NRS_NIGHT_SKY
    if (q > 0.75) {
      col += nrsNightSky(d, S, time, rain);
    }
  #endif

  #if NRS_AURORA
    if (q > 0.75) {
      col += nrsAurora(d, S, time, rain);
    }
  #endif

  #if NRS_CLOUDS
    if (d.y > -0.02) {
      col = nrsCloudLayers(col, d, S, L, Lcol, nrsAmbientColor(S.y, rain), time, q, rain);
    }
  #endif

  float l = nrsLuma(col);
  return mix(vec3_splat(l), col, NRS_SKY_SATURATION);
}

#endif
