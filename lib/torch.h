#ifndef NRS_TORCH_H
#define NRS_TORCH_H

// Torch, lantern and fire light: warm light that is strong next to the source
// and falls off like light in the real world (one over distance squared).

#include "nrs_math.h"

// the game light map is level/15, the curve below wants level/16
vec2 nrsLightUv(vec2 uv1) {
  return uv1*(15.0/16.0);
}

// strength of the block light, uv1x is the block light level/16
float nrsTorchCurve(float uv1x) {
  float level = uv1x*16.0;
  // distance from the source in blocks, a source gives level 14 next to it
  float dist = max(14.0 - level, 0.0) + 0.55;
  float fall = 1.0/(dist*dist);
  // fades out completely at the lowest levels
  return fall*saturate(level*(1.0/3.0));
}

// colour and strength of the block light
vec3 nrsTorchLight(vec2 uv1) {
  return nrsToLinear(NRS_TORCH_COLOR)*(NRS_TORCH_POWER*nrsTorchCurve(uv1.x));
}

#endif
