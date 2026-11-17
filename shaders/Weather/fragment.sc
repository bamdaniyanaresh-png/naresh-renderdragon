$input v_light, v_occlusion, v_uv, v_uv1

#include <bgfx_shader.sh>
#include <naresh.sh>

uniform vec4 OcclusionHeightOffset;

SAMPLER2D_AUTOREG(s_LightingTexture);
SAMPLER2D_AUTOREG(s_OcclusionTexture);
SAMPLER2D_AUTOREG(s_WeatherTexture);

bool isOccluded(vec2 uv, float height, float limit) {
  #ifdef NO_OCCLUSION
    return false;
  #else
    bool inside = uv.x >= 0.0 && uv.x <= 1.0 && uv.y >= 0.0 && uv.y <= 1.0;
    #ifdef FLIP_OCCLUSION
      bool covered = height > limit;
    #else
      bool covered = height < limit;
    #endif
    return inside && covered;
  #endif
}

void main() {
  vec4 color = texture2D(s_WeatherTexture, v_uv);
  vec4 occ = texture2D(s_OcclusionTexture, v_occlusion.xy);

  // the occlusion texture holds the height of the roof and its light level
  float roofLight = fract(occ.z*16.0);
  float roofHeight = (0.25*occ.x + fract(64.0*occ.y))*1024.0;
  roofHeight = (roofHeight + OcclusionHeightOffset.x)/255.0;

  vec2 lightUV = vec2(0.0, 0.0);
  if (!isOccluded(v_occlusion.xy, v_occlusion.z, roofHeight)) {
    float blend = clamp((v_occlusion.z - roofHeight)*25.0, 0.0, 1.0);
    lightUV = vec2(roofLight*(1.0 - blend), 1.0);
  }

  vec3 lightTex = texture2D(s_LightingTexture, lightUV).rgb;

  // v_light.rgb is the sky and sun light that falls on the drops
  color.rgb = nrsTone(nrsToLinear(color.rgb)*lightTex*v_light.rgb);
  color.a *= lightUV.y*(1.0 - v_light.a);

  gl_FragColor = color;
}
