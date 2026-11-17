#ifndef NRS_STARS_H
#define NRS_STARS_H

// naresh gaming creation
// Stars of three sizes. The sky is cut into rows of cells (more cells per row near the horizon,
// fewer near the top), so a star has the same size and the same density everywhere.
// Used by the night sky in sky.h.

#include "nrs_math.h"

// one layer of stars. lat is the height angle of the ray, az the turn around (0 to 1).
// tint is a random number of the star that picks its colour.
highp float nrsStarLayer(
  highp float lat, highp float az, float scale, float threshold, float speed, float seed,
  highp float time, out float tint
) {
  highp float row = floor(lat*scale);
  highp float latc = (row + 0.5)/scale;
  highp float n = max(floor(2.0*NRS_PI*scale*cos(latc)), 1.0);
  highp float gx = az*n;
  highp float col = floor(gx);
  highp vec2 id = vec2(col, row);

  float r = nrsHash(id + seed);
  tint = 0.5;

  // weak stars are removed: they are too small to see and only cost time
  float cut = threshold + NRS_STAR_DIM_CUT*(1.0 - threshold);
  if (r <= cut) {
    return 0.0;
  }

  tint = nrsHash(id + 0.5);
  vec2 f = vec2(gx - col - 0.5, lat*scale - row - 0.5);

  // the star sits somewhere inside its cell
  vec2 o = vec2(nrsHash(id + 1.37), nrsHash(id + 8.21)) - 0.5;
  float dd = length(f - o*0.7);
  float s = smoothstep(0.13, 0.0, dd);
  float tw = 0.7 + 0.3*sin(time*speed + r*6.28318);
  return s*tw*(r - cut)/((1.0 - NRS_STAR_DIM_CUT)*(1.0 - threshold));
}

// all stars along a ray. boost makes the big ones brighter (inside the band of the night sky).
highp vec3 nrsStars(highp vec3 d, highp float time, float boost) {
  // the angles are worked out once for the three layers
  highp float lat = asin(clamp(d.y, -1.0, 1.0));
  highp float az = atan(d.x, d.z)*(0.5/NRS_PI) + 0.5;

  float t1;
  float t2;
  float t3;

  float s1 = nrsStarLayer(lat, az, 110.0, NRS_STAR_SMALL, 2.0, 13.1, time, t1);
  highp vec3 col = vec3(0.90, 0.93, 1.0)*(s1*(0.85 + boost*1.8));

  float s2 = nrsStarLayer(lat, az, 55.0, 0.955, 2.6, 47.3, time, t2);
  col += mix(vec3(1.0, 0.75, 0.58), vec3(0.78, 0.88, 1.0), t2)*(s2*1.15);

  float s3 = nrsStarLayer(lat, az, 26.0, 0.984, 3.2, 89.7, time, t3);
  col += mix(vec3(1.0, 0.65, 0.45), vec3(0.85, 0.95, 1.0), t3)*(s3*1.7);

  return col;
}

#endif
