#ifndef NRS_TORNADO_H
#define NRS_TORNADO_H

// A turning storm funnel in the End.
// The funnel is a few thin cone walls. The ray of every pixel is crossed with each wall by one
// small equation (no marching), and the swirl is a cheap noise on the wall. So it costs little.

#include "nrs_math.h"

#if NRS_TORNADO

// value noise that repeats every `period` cells along x, so the pattern closes around the funnel
highp float nrsNoiseRing(highp vec2 p, float period) {
  highp vec2 i = floor(p);
  highp vec2 f = fract(p);
  f = f*f*(3.0 - 2.0*f);
  highp float x0 = mod(i.x, period);
  highp float x1 = mod(i.x + 1.0, period);
  highp float a = nrsHash(vec2(x0, i.y));
  highp float b = nrsHash(vec2(x1, i.y));
  highp float c = nrsHash(vec2(x0, i.y + 1.0));
  highp float d = nrsHash(vec2(x1, i.y + 1.0));
  return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// swirl on a wall: streaks that wind upwards and turn with time. 0 to 1.
// far = 1 for parts that are far away (one noise is enough there)
highp float nrsTornadoSwirl(highp vec3 P, highp float time, float spin, float far) {
  highp float turn = atan(P.z, P.x)*(0.5/NRS_PI) + 0.5;
  highp float h = (P.y - NRS_TORNADO_BASE)/(NRS_TORNADO_TOP - NRS_TORNADO_BASE);
  const float ring = 6.0;
  // the streaks turn faster near the bottom
  highp float sx = turn*ring - h*2.6 - time*NRS_TORNADO_SPEED*(0.10 + 0.16*(1.0 - saturate(h)))*spin;
  highp float sy = h*9.0*(1.0 + 0.3*spin);
  highp float n = nrsNoiseRing(vec2(sx, sy), ring);
  if (far < 0.5) {
    n = 0.62*n + 0.38*nrsNoiseRing(vec2(sx*2.0 + 1.7, sy*2.0 + 3.1), ring*2.0);
  }
  return n;
}

// one point where the ray meets a wall. T is what is left of the light behind it,
// acc the colour of the wall times its coverage, asum the sum of the coverages.
void nrsTornadoHit(
  highp vec3 o, highp vec3 d, highp float t, float rad, float slope, float gain, float tintMix,
  float spin, highp float time, inout float T, inout vec3 acc, inout float asum
) {
  highp vec3 P = o + d*t;
  highp float h = (P.y - NRS_TORNADO_BASE)/(NRS_TORNADO_TOP - NRS_TORNADO_BASE);
  if (t <= 0.0 || h <= 0.0 || h >= 1.0 || rad <= 0.0) {
    return;
  }

  highp float n = nrsTornadoSwirl(P, time, spin, t > 400.0 ? 1.0 : 0.0);
  float dens = smoothstep(0.42, 0.80, n)*gain*NRS_TORNADO_DENSITY;

  // the bottom and the top fade out
  float hf = smoothstep(0.0, 0.10, h)*(1.0 - smoothstep(0.82, 1.0, h));

  // the edge seen from the side is thin and soft
  highp float rl = max(length(P.xz), 0.001);
  highp vec3 Nn = normalize(vec3(P.x/rl, -slope, P.z/rl));
  hf *= smoothstep(0.05, 0.55, abs(dot(Nn, d)));

  float al = clamp(dens*hf*0.55, 0.0, 0.95)*exp(-min(t, 5000.0)/NRS_TORNADO_FADE);
  if (al < 0.003) {
    return;
  }

  // dark grey blue in the gaps, light on the dense streaks, teal near the top
  vec3 base = mix(vec3(0.05, 0.07, 0.12), vec3(0.50, 0.60, 0.75), saturate(n*1.3));
  vec3 teal = vec3(0.18, 0.55, 0.75)*(saturate(h)*saturate(h)*0.9);
  vec3 c = base + teal*(0.5 + tintMix);

  T *= 1.0 - al;
  acc += c*al;
  asum += al;
}

// one cone wall: radius = r0*scale + slope*(height - base)
void nrsTornadoWall(
  highp vec3 o, highp vec3 d, float scale, float gain, float tintMix, float spin, highp float time,
  inout float T, inout vec3 acc, inout float asum
) {
  float slope = scale*(NRS_TORNADO_R1 - NRS_TORNADO_R0)/(NRS_TORNADO_TOP - NRS_TORNADO_BASE);
  highp float c0 = NRS_TORNADO_R0*scale + slope*(o.y - NRS_TORNADO_BASE);
  highp float c1 = slope*d.y;

  // |o.xz + t*d.xz| = c0 + c1*t  is a quadratic equation in t
  highp float a = dot(d.xz, d.xz) - c1*c1;
  highp float b = 2.0*(dot(o.xz, d.xz) - c0*c1);
  highp float c = dot(o.xz, o.xz) - c0*c0;
  highp float disc = b*b - 4.0*a*c;
  if (disc <= 0.0 || abs(a) < 0.00001) {
    return;
  }
  highp float sq = sqrt(disc);
  highp float t1 = (-b - sq)/(2.0*a);
  highp float t2 = (-b + sq)/(2.0*a);

  nrsTornadoHit(o, d, t1, c0 + c1*t1, slope, gain, tintMix, spin, time, T, acc, asum);
  nrsTornadoHit(o, d, t2, c0 + c1*t2, slope, gain, tintMix, spin, time, T, acc, asum);
}

// the funnel along a ray. sky is the light behind it, cam the world position of the camera,
// d the view direction. Returns the new light.
highp vec3 nrsTornado(highp vec3 sky, highp vec3 cam, highp vec3 d, highp float time) {
  highp vec3 o = cam - vec3(NRS_TORNADO_X, 0.0, NRS_TORNADO_Z);

  // far away the funnel is not worked out
  if (dot(o.xz, o.xz) > NRS_TORNADO_RANGE*NRS_TORNADO_RANGE) {
    return sky;
  }

  float T = 1.0;
  vec3 acc = vec3_splat(0.0);
  float asum = 0.0;

  nrsTornadoWall(o, d, 1.0, 1.0, 0.0, 1.0, time, T, acc, asum);
  nrsTornadoWall(o, d, 0.55, 1.25, 0.4, 1.4, time, T, acc, asum);
  #if NRS_TORNADO_OUTER
    nrsTornadoWall(o, d, 1.55, 0.7, -0.3, 1.8, time, T, acc, asum);
  #endif

  // order does not matter: the walls are mixed by their coverage
  vec3 avg = acc/max(asum, 0.0001);
  highp vec3 outc = sky*T + avg*(1.0 - T);

  // glowing eye near the top: distance of the ray from one point
  highp vec3 p = vec3(0.0, NRS_TORNADO_TOP - 40.0, 0.0) - o;
  highp float tt = max(dot(d, p), 0.0);
  highp vec3 q = p - d*tt;
  float dist2 = dot(q, q);
  float glow = exp(-dist2*(1.0/(55.0*55.0)))*0.9 + exp(-dist2*(1.0/(160.0*160.0)))*0.10;
  outc += vec3(0.20, 0.62, 0.85)*(glow*1.1*NRS_TORNADO_GLOW);

  return outc;
}

#endif

#endif
