#ifndef NRS_TONE_H
#define NRS_TONE_H

// Tone mapping. Four curves are mixed, the soft exponential one is the main one.
// All curves are written as plain maths on the colour, there is no lookup table.

#include "nrs_math.h"

// 1: soft exponential, deep shadows and a long smooth shoulder
vec3 nrsCurveSoft(vec3 c) {
  return vec3_splat(1.0) - exp(-0.95*c);
}

// 2: rational filmic curve with a toe
vec3 nrsCurveFilmic(vec3 c) {
  vec3 x = c*1.25;
  vec3 n = x*(0.22*x + 0.05) + 0.004;
  vec3 d = x*(0.22*x + 0.50) + 0.06;
  return (n/d - 0.004/0.06)*1.45;
}

// 3: cinematic curve, bright highlights
vec3 nrsCurveCine(vec3 c) {
  vec3 y = c*0.9;
  return (y*(2.51*y + 0.03))/(y*(2.43*y + 0.59) + 0.14);
}

// 4: Reinhard with a white point, lifts the shadows
vec3 nrsCurveReinhard(vec3 c) {
  vec3 r = c*0.8;
  return r*(1.0 + r*(1.0/9.0))/(1.0 + r);
}

vec3 nrsToneMix(vec3 c) {
  c = max(c, vec3_splat(0.0));
  float total = NRS_TM_SOFT + NRS_TM_FILMIC + NRS_TM_ACES + NRS_TM_REINHARD;
  vec3 m = NRS_TM_SOFT*nrsCurveSoft(c) + NRS_TM_FILMIC*nrsCurveFilmic(c)
         + NRS_TM_ACES*nrsCurveCine(c) + NRS_TM_REINHARD*nrsCurveReinhard(c);
  return clamp(m/total, 0.0, 1.0);
}

// saturation, S curve and warmth on the display colour
vec3 nrsGrade(vec3 c) {
  float l = dot(c, vec3(0.299, 0.587, 0.114));
  c = clamp(mix(vec3_splat(l), c, NRS_TM_SATURATION), 0.0, 1.0);
  c = mix(c, c*c*(3.0 - 2.0*c), NRS_TM_CONTRAST);
  c *= vec3(1.0 + NRS_TM_WARMTH, 1.0, 1.0 - NRS_TM_WARMTH);
  return clamp(c, 0.0, 1.0);
}

// linear light -> display colour
vec3 nrsTone(vec3 lig) {
  return nrsGrade(nrsToGamma(nrsToneMix(lig*NRS_EXPOSURE)));
}

#endif
