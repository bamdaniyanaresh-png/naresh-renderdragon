#ifndef NRS_MATH_H
#define NRS_MATH_H

// Math helpers: colour space, hash and value noise.

#include "settings.h"

#define NRS_PI 3.14159265

float nrsSq(float x) {
  return x*x;
}

float nrsLuma(vec3 c) {
  return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

vec3 nrsToLinear(vec3 c) {
  return pow(max(c, vec3_splat(0.0)), vec3_splat(NRS_GAMMA));
}

vec3 nrsToGamma(vec3 c) {
  float g = 1.0/NRS_GAMMA;
  return pow(max(c, vec3_splat(0.0)), vec3_splat(g));
}

// hash of a 2d point, 0 to 1 (no sin, so it is the same on every GPU)
highp float nrsHash(highp vec2 p) {
  highp vec3 p3 = fract(vec3(p.x, p.y, p.x)*vec3(0.1139, 0.1071, 0.0953));
  p3 += dot(p3, p3.yzx + 31.7);
  return fract((p3.x + p3.y)*p3.z);
}

// smooth value noise, 0 to 1
highp float nrsNoise(highp vec2 p) {
  highp vec2 i = floor(p);
  highp vec2 f = fract(p);
  f = f*f*(3.0 - 2.0*f);
  highp float a = nrsHash(i);
  highp float b = nrsHash(i + vec2(1.0, 0.0));
  highp float c = nrsHash(i + vec2(0.0, 1.0));
  highp float d = nrsHash(i + vec2_splat(1.0));
  return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// sum of noise octaves, the result keeps the same mean for every octave count (max 5)
highp float nrsFbm(highp vec2 p, float octaves) {
  highp float s = 0.0;
  highp float a = 0.5;
  highp float w = 0.0;
  for (float i = 0.0; i < 5.0; i++) {
    if (i >= octaves) {
      break;
    }
    s += a*nrsNoise(p);
    w += a;
    p = vec2(0.8*p.x - 0.6*p.y, 0.6*p.x + 0.8*p.y)*2.02 + vec2(7.3, 3.1);
    a *= 0.5;
  }
  return s*(0.9375/w);
}

#endif
