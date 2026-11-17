#ifndef NRS_ENTITY_H
#define NRS_ENTITY_H

// Lighting of mobs, held items and weather. Same sun, sky light and fog as the world.

#include "terrain.h"

// worldPos  camera relative position
// N         world space normal
// tile      TileLightColor (r = block light, b = sky light, level/15)
// lit       light that multiplies the linear colour, fogCol/fogAmt the air in front
void nrsEntityVertex(
  float tod, float rain, highp vec3 worldPos, highp vec3 N, vec4 tile,
  out vec3 lit, out vec3 fogCol, out float fogAmt
) {
  highp vec3 S = nrsSunTrue(tod);
  highp vec3 L;
  vec3 Lcol;
  nrsLightFromSun(S, rain, L, Lcol);

  vec2 uv1 = nrsLightUv(vec2(tile.r, tile.b));
  float skyL = nrsSkyCurve(uv1.y);
  float shadow = nrsShadowMask(uv1.y);

  vec3 sunCol = Lcol*NRS_SUN_TINT;
  vec3 ambient = nrsAmbientColor(S.y, rain)*NRS_AMBIENT_TINT*NRS_AMBIENT_POWER;

  vec3 sun = sunCol*(saturate(dot(N, L))*shadow);
  vec3 sky = mix(ambient*vec3(0.42, 0.40, 0.38), ambient, N.y*0.5 + 0.5)*skyL;
  vec3 torch = nrsTorchLight(uv1);

  lit = sun + sky + torch + vec3(0.012, 0.014, 0.020);

  highp float dist = length(worldPos);
  fogCol = nrsHorizonColor(worldPos/max(dist, 0.001), S, rain)*(0.04 + 0.96*skyL);
  fogAmt = nrsFogAmount(dist, rain);
}

// linear lit colour -> display colour
vec3 nrsEntityFinish(vec3 color, vec3 fogCol, float fogAmt) {
  return nrsTone(mix(color, fogCol, fogAmt));
}

// light that falls on rain and snow
void nrsWeatherLight(float tod, float rain, out vec3 lig) {
  highp vec3 S = nrsSunTrue(tod);
  highp vec3 L;
  vec3 Lcol;
  nrsLightFromSun(S, rain, L, Lcol);
  lig = nrsAmbientColor(S.y, rain)*NRS_AMBIENT_POWER + Lcol*0.12;
}

#endif
