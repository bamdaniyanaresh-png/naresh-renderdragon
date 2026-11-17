$input a_color0, a_position, a_texcoord0, a_texcoord1
#ifdef INSTANCING
  $input i_data0, i_data1, i_data2, i_data3
#endif
$output v_tint, v_vcol, v_ambient, v_sun, v_fog, v_uv, v_light, v_wpos

#include <bgfx_shader.sh>
#include <naresh.sh>

uniform vec4 ViewPositionAndTime;
uniform vec4 TimeOfDay;
uniform vec4 FogAndDistanceControl;

void main() {
  #ifdef INSTANCING
    mat4 model = mtxFromCols(i_data0, i_data1, i_data2, i_data3);
  #else
    mat4 model = u_model[0];
  #endif

  vec3 wpos = mul(model, vec4(a_position, 1.0)).xyz;

  #if !(defined(DEPTH_ONLY_OPAQUE) || defined(DEPTH_ONLY) || defined(INSTANCING))

  #ifdef RENDER_AS_BILLBOARDS
    // far chunks are drawn as quads that face the camera
    wpos += vec3(0.5, 0.5, 0.5);

    vec3 toCam = ViewPositionAndTime.xyz - wpos;
    vec3 viewDir = toCam/length(toCam);

    vec3 side = normalize(vec3(-viewDir.z, 0.0, viewDir.x));
    wpos -= (viewDir.zxy*side.yzx - viewDir.yzx*side.zxy)*(a_color0.z - 0.5) + side*(a_color0.x - 0.5);
    vec4 vcol = vec4(1.0, 1.0, 1.0, 1.0);
  #else
    vec4 vcol = a_color0;
  #endif

  // texture coordinates
  vec2 uv = 2.0*a_texcoord0.xy;
  uv = fract(uv) + (floor(uv) - 0.5)/16384.0;

  // light map, block light in bits 8-5 and sky light in bits 4-1 (level/15)
  vec2 lightUV = fract(a_texcoord1.y*vec2(256.0, 4096.0));
  vec2 light = nrsLightUv(lightUV);

  // biome tint and shade of the face
  #ifdef SEASONS
    vec4 tint = vec4(1.0, 1.0, 1.0, vcol.a);
  #else
    vec4 tint = nrsVertexTint(vcol);
  #endif

  vec3 ambient;
  vec3 sunCol;
  vec3 fogCol;
  float fogAmt;
  float rain = nrsRainAmount(FogAndDistanceControl);
  nrsTerrainVertex(TimeOfDay.x, rain, wpos, light, ambient, sunCol, fogCol, fogAmt);

  // water has a blue vertex colour with some transparency
  float water = 0.0;
  #if defined(TRANSPARENT) && !(defined(RENDER_AS_BILLBOARDS) || defined(SEASONS))
    if (a_color0.b > 0.3 && a_color0.a < 0.95) {
      water = 1.0;
    }
  #endif

  v_tint = tint;
  v_vcol = vcol;
  v_ambient = vec4(ambient, water);
  v_sun = vec4(sunCol, rain);
  #if NRS_RAIN_DEBUG
    v_fog = vec4(FogAndDistanceControl.x, FogAndDistanceControl.y, FogAndDistanceControl.z/512.0, 0.0);
  #else
    v_fog = vec4(fogCol, fogAmt);
  #endif
  v_uv = uv;
  v_light = lightUV;
  v_wpos = wpos;

  #endif

  gl_Position = mul(u_viewProj, vec4(wpos, 1.0));
}
