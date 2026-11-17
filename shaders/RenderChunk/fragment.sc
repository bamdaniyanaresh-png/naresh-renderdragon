$input v_tint, v_vcol, v_ambient, v_sun, v_fog, v_uv, v_light, v_wpos

#include <bgfx_shader.sh>
#include <naresh.sh>

uniform vec4 TimeOfDay;
uniform vec4 ViewPositionAndTime;
uniform vec4 CameraPosition;

SAMPLER2D_AUTOREG(s_MatTexture);
SAMPLER2D_AUTOREG(s_SeasonsTexture);
SAMPLER2D_AUTOREG(s_LightMapTexture);

void main() {
  #if defined(DEPTH_ONLY_OPAQUE) || defined(DEPTH_ONLY) || defined(INSTANCING)
    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    return;
  #endif

  vec4 tex = texture2D(s_MatTexture, v_uv);

  #ifdef ALPHA_TEST
    if (tex.a < 0.6) {
      discard;
    }
  #endif

  vec4 tint = v_tint;
  #if defined(SEASONS) && (defined(OPAQUE) || defined(ALPHA_TEST))
    tint.rgb = mix(vec3(1.0, 1.0, 1.0), texture2D(s_SeasonsTexture, v_vcol.xy).rgb*2.0, v_vcol.z);
  #endif

  // surface normal from the screen space derivatives, turned to face the camera
  highp vec3 wpos = v_wpos;
  highp vec3 dpx = dFdx(wpos);
  highp vec3 dpy = dFdy(wpos);
  highp vec3 normal = normalize(cross(dpx, dpy));
  if (dot(normal, wpos) > 0.0) {
    normal = -normal;
  }
  vec2 dux = dFdx(v_uv);
  vec2 duy = dFdy(v_uv);

  highp vec3 gpos = wpos + CameraPosition.xyz;
  vec2 light = nrsLightUv(v_light);

  vec4 color = nrsTerrainShade(
    tex, v_vcol, tint, light, wpos, gpos, normal, dpx, dpy, dux, duy,
    v_ambient.rgb, v_sun.rgb, v_fog.rgb, v_fog.a,
    v_ambient.w > 0.5, TimeOfDay.x, v_sun.w, ViewPositionAndTime.w
  );

  #if NRS_RAIN_DEBUG
    color = vec4(v_fog.rgb, 1.0);
  #endif

  #if NRS_CHUNK_BOUND || NRS_LIGHT_MAP
    vec3 blockPos = fract(gpos);
  #endif
  #if NRS_CHUNK_BOUND
    color.rgb = nrsChunkBound(color.rgb, mod(gpos, 16.0), normal, blockPos);
  #endif
  #if NRS_LIGHT_MAP == 1
    color.rgb = nrsLightBound(color.rgb, light, normal, blockPos);
  #endif

  gl_FragColor = color;
}
