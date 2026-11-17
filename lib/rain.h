#ifndef NRS_RAIN_H
#define NRS_RAIN_H

// Rain: how much it rains, wet ground patches and the rain drop rings.

#include "nrs_math.h"

// 0 = dry, 1 = full rain. The game shortens the fog while it rains, so the fog start tells the weather.
// fogCtrl is FogAndDistanceControl.
float nrsRainAmount(vec4 fogCtrl) {
  #if !NRS_RAIN
    return 0.0;
  #elif NRS_RAIN_TEST
    return 1.0;
  #else
    float r = 1.0 - smoothstep(NRS_RAIN_FOG_WET, NRS_RAIN_FOG_CLEAR, fogCtrl.x);
    // under water and in other dimensions the fog start is near 0, that is not rain
    return r*smoothstep(0.10, 0.20, fogCtrl.x);
  #endif
}

// patches of the ground that hold water, 0 to 1
float nrsWetMask(highp vec2 gxz) {
  float n = nrsNoise(gxz*0.35)*0.65 + nrsNoise(gxz*0.9 + 17.0)*0.35;
  return smoothstep(0.38, 0.62, n);
}

// rain amount for the sky and for mobs, they can be switched off in settings.h
#if NRS_ENTITY_RAIN
  #define NRS_ENTITY_RAIN_AMOUNT nrsRainAmount(FogAndDistanceControl)
#else
  #define NRS_ENTITY_RAIN_AMOUNT 0.0
#endif

#if NRS_SKY_RAIN
  #define NRS_SKY_RAIN_AMOUNT nrsRainAmount(FogAndDistanceControl)
#else
  #define NRS_SKY_RAIN_AMOUNT 0.0
#endif

#if NRS_RAIN && NRS_RAIN_RIPPLE

// random start time of a drop in a cell
float nrsRippleTime(highp vec2 cell) {
  return nrsHash(cell + vec2(3.7, 9.1));
}

// random place of the drop inside its cell
highp vec2 nrsRippleSpot(highp vec2 cell) {
  return vec2(nrsHash(cell), nrsHash(cell + vec2(41.3, 17.9)));
}

// normal of the ring field at a world position: xy is the slope on the ground plane, z is up
vec3 nrsRainRipple(highp vec2 worldXZ, highp float t) {
  highp vec2 p3 = worldXZ*NRS_RAIN_RIPPLE_SCALE;
  highp vec2 p0 = floor(p3);
  vec2 circles = vec2_splat(0.0);

  for (float j = -float(NRS_RAIN_RIPPLE_RADIUS); j <= float(NRS_RAIN_RIPPLE_RADIUS); j++) {
    for (float i = -float(NRS_RAIN_RIPPLE_RADIUS); i <= float(NRS_RAIN_RIPPLE_RADIUS); i++) {
      highp vec2 cell = p0 + vec2(i, j);
      highp vec2 p = cell + nrsRippleSpot(cell);
      float tt = fract(NRS_RAIN_RIPPLE_SPEED*t + nrsRippleTime(cell));
      vec2 v = p - p3;
      float len = max(length(v), 0.0001);
      // the ring grows with the age of the drop
      float d = len - (float(NRS_RAIN_RIPPLE_RADIUS) + 1.0)*tt;
      float h = 0.001;
      float d1 = d - h;
      float d2 = d + h;
      float w1 = sin(31.0*d1)*smoothstep(-0.6, -0.3, d1)*smoothstep(0.0, -0.3, d1);
      float w2 = sin(31.0*d2)*smoothstep(-0.6, -0.3, d2)*smoothstep(0.0, -0.3, d2);
      float fade = 1.0 - tt;
      circles += 0.5*(v/len)*((w2 - w1)/(2.0*h))*(fade*fade);
    }
  }

  float cells = float((NRS_RAIN_RIPPLE_RADIUS*2 + 1)*(NRS_RAIN_RIPPLE_RADIUS*2 + 1));
  circles /= cells;

  // keep the square root valid
  float cl = length(circles);
  circles *= min(1.0, 0.95/max(cl, 0.0001));

  return vec3(circles, sqrt(1.0 - dot(circles, circles)));
}

// bright rings and dark rings on a wet surface, returns vec4(rgb light, darkening)
// strength: rain * wet patch * exposure to the sky, brightness: day or torch light
vec4 nrsRainRippleShade(vec3 n, float strength, float brightness) {
  float spec = pow(saturate(dot(n, normalize(vec3(1.0, 0.7, 0.5)))), 6.0);
  float light = 5.0*spec*brightness*strength*NRS_RAIN_RIPPLE_POWER;
  float darken = min(length(n.xy)*0.6, 0.4)*strength;
  return vec4(light, light, light, darken);
}

#endif

#endif
