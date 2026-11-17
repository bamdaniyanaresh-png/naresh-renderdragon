#ifndef NRS_CLOUDS_H
#define NRS_CLOUDS_H

// Two cloud layers on flat planes: soft puffy clouds with their own shadow and a silver edge,
// and thin high clouds. Called from nrsSkyFull in sky.h.

#include "daylight.h"

#if NRS_CLOUDS

// thickness of the soft clouds, 0 to 1
highp float nrsCloudDensity(highp vec2 p, float cover) {
  // a slow warp breaks the regular look of the noise
  highp vec2 w = vec2(nrsNoise(p*0.7 + 11.0), nrsNoise(p*0.7 + 37.0)) - 0.5;
  p += w*0.55;
  highp float n = nrsFbm(p, NRS_CLOUD_OCTAVES);
  return smoothstep(cover - 0.04, cover + 0.30, n);
}

highp vec3 nrsCloudLayers(
  highp vec3 sky, highp vec3 d, highp vec3 S, highp vec3 L, vec3 Lcol, vec3 amb,
  highp float time, float q, float rain
) {
  float dy = d.y;
  // rain closes the sky: more clouds
  float cover = NRS_CLOUD_COVER - 0.28*rain;
  highp vec3 outc = sky;

  // soft clouds. The plane is seen from below, so the horizon is squeezed
  highp float t = 1.0/(max(dy, 0.0) + 0.22);
  highp vec2 p = d.xz*t*NRS_CLOUD_SCALE + time*NRS_CLOUD_SPEED;
  highp float d1 = nrsCloudDensity(p, cover);

  float cosl = saturate(dot(d, L));
  highp vec3 cloud = vec3_splat(0.0);
  float alpha = 0.0;

  if (d1 > 0.002) {
    float trans = 0.7;
    if (q > 0.75) {
      // thickness towards the light, a low light looks further along the layer
      highp vec2 toL = L.xz/max(L.y, 0.3);
      float s1 = nrsCloudDensity(p + toL*0.22, cover);
      float s2 = nrsCloudDensity(p + toL*0.50, cover);
      trans = exp(-1.7*(s1 + s2*0.8));
    }
    float silver = pow(cosl, 5.0)*(1.0 - d1)*1.2;
    vec3 lit = Lcol*(trans*(0.85 + silver*1.6));
    vec3 ambc = amb*((0.55 + 0.45*(1.0 - d1))*0.95);
    cloud = ambc + lit*0.9;

    // far clouds fade into the colour of the horizon
    // rain clouds are grey
    cloud = mix(cloud, vec3_splat(nrsLuma(cloud))*vec3(0.95, 0.98, 1.05), rain*0.6);

    highp vec3 horc = nrsSkyBase(normalize(vec3(d.x, 0.02, d.z)), S, rain);
    float ap = pow(saturate(1.0 - dy*1.6), 3.0)*0.6;
    cloud = mix(cloud, horc*1.05, ap);

    alpha = (1.0 - exp(-3.2*d1))*smoothstep(0.0, 0.14, dy)*0.95;
    outc = mix(outc, cloud, alpha);
  }

  #if NRS_CIRRUS
    if (q > 0.75) {
      // thin high clouds, stretched along one direction
      highp float t2 = 1.0/(max(dy, 0.0) + 0.30);
      highp vec2 c2 = vec2(d.x*t2*0.35 + time*0.01, d.z*t2*1.2 + time*0.005);
      float n2 = nrsFbm(c2, 4.0);
      float ci = smoothstep(0.50, 0.78, n2)*0.38*smoothstep(0.02, 0.25, dy);
      vec3 cic = amb*0.9 + Lcol*(0.5*(0.6 + 0.8*cosl));
      outc = mix(outc, cic, ci*(1.0 - alpha*0.7));
    }
  #endif

  return outc;
}

#else

highp vec3 nrsCloudLayers(
  highp vec3 sky, highp vec3 d, highp vec3 S, highp vec3 L, vec3 Lcol, vec3 amb,
  highp float time, float q, float rain
) {
  return sky;
}

#endif

#endif
