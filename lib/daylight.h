#ifndef NRS_DAYLIGHT_H
#define NRS_DAYLIGHT_H

// Time of day: sun and moon direction, the colour of their light and the sky light.
// TimeOfDay.x is 0 at noon, 0.25 at sunset, 0.5 at midnight and 0.75 at sunrise.

#include "nrs_math.h"

float nrsDayAmt(float sy) {
  return smoothstep(-0.12, 0.28, sy);
}

float nrsDuskAmt(float sy) {
  return exp(-nrsSq((sy - 0.02)*4.5));
}

// direction of the sun, it is below the horizon at night
highp vec3 nrsSunTrue(float tod) {
  float a = 2.0*NRS_PI*tod;
  highp vec3 s = vec3(-sin(a), cos(a), 0.0);
  // the sun path is tilted a little
  const float cl = 0.96593;
  const float sl = 0.25882;
  float y = s.y;
  float z = s.z;
  s.y = cl*y - sl*z;
  s.z = sl*y + cl*z;
  s.x += 0.00001;
  return normalize(s);
}

// light of the sun, warm and weak when it is low
vec3 nrsSunColor(float sy) {
  float t = smoothstep(0.0, 0.5, sy);
  vec3 c = mix(vec3(1.0, 0.38, 0.12), vec3(1.10, 0.98, 0.84), t);
  return c*(NRS_SUN_POWER*smoothstep(-0.04, 0.16, sy));
}

vec3 nrsMoonColor(float sy) {
  return vec3(0.52, 0.66, 1.0)*(NRS_MOON_POWER*smoothstep(0.04, 0.24, -sy));
}

// direct light: the sun by day and the moon by night. Both are 0 where they switch.
void nrsLightFromSun(highp vec3 S, float rain, out highp vec3 L, out vec3 Lcol) {
  if (S.y > -0.04) {
    L = S;
    Lcol = nrsSunColor(S.y);
  } else {
    L = -S;
    Lcol = nrsMoonColor(S.y);
  }
  // in rain the sun and the moon fade away
  Lcol *= 1.0 - NRS_RAIN_SUN_HIDE*rain;
}

// colour of the light that comes from the whole sky
vec3 nrsAmbientColor(float sy, float rain) {
  float day = nrsDayAmt(sy);
  float dusk = nrsDuskAmt(sy);
  vec3 a = mix(vec3(0.075, 0.115, 0.26), vec3(0.46, 0.62, 0.92)*0.85, day);
  a = mix(a, vec3(0.55, 0.36, 0.40)*0.55, dusk*0.6);
  // in rain the sky light is grey and a little darker
  a = mix(a, vec3_splat(nrsLuma(a))*vec3(0.96, 1.0, 1.06), rain*0.55);
  // overcast light is soft and comes from everywhere, so the sky light grows when the sun is gone
  return a*((1.0 + rain)*(1.0 - NRS_RAIN_DARKEN*rain));
}

#endif
