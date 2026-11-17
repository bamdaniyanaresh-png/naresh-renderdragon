#ifndef NRS_ENDSKY_H
#define NRS_ENDSKY_H

// Colour of the End sky: the texture of the game turned to a cold blue, with a soft haze.

#include "nrs_math.h"

// texLinear is the sky texture as linear light, d the view direction
vec3 nrsEndSky(vec3 texLinear, highp vec3 d) {
  float l = nrsLuma(texLinear);
  vec3 tint = NRS_END_SKY_TINT/nrsLuma(NRS_END_SKY_TINT);
  vec3 c = mix(texLinear, vec3_splat(l)*tint, NRS_END_SKY_TINT_AMOUNT);
  return c + NRS_END_HAZE*(0.45 + 0.55*(1.0 - abs(d.y)));
}

#endif
