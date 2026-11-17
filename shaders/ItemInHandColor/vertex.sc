$input a_position, a_color0, a_texcoord0, a_indices, a_normal
#ifdef INSTANCING
  $input i_data0, i_data1, i_data2
#endif
$output v_vcol, v_lit, v_atten

#include <bgfx_shader.sh>
#include <MinecraftRenderer.Materials/TAAUtil.dragonh>
#include <naresh.sh>

uniform vec4 TileLightColor;
uniform vec4 TimeOfDay;
#if NRS_ENTITY_RAIN
  uniform vec4 FogAndDistanceControl;
#endif

void main() {
  mat4 world = u_model[0];

  vec3 wpos;
  #ifdef INSTANCING
    mat4 inst = mtxFromCols(i_data0, i_data1, i_data2, vec4(0.0, 0.0, 0.0, 1.0));
    wpos = instMul(inst, vec4(a_position, 1.0)).xyz;
  #else
    wpos = mul(world, vec4(a_position, 1.0)).xyz;
  #endif

  vec4 clip = jitterVertexPosition(wpos);

  #if !(defined(DEPTH_ONLY) || defined(INSTANCING))
    vec3 normal = normalize(mul(world, a_normal)).xyz;

    vec3 lit;
    vec3 fogCol;
    float fogAmt;
    nrsEntityVertex(TimeOfDay.x, NRS_ENTITY_RAIN_AMOUNT, wpos, normal, TileLightColor, lit, fogCol, fogAmt);

    v_vcol = a_color0;
    v_lit = vec4(lit, 1.0);
    v_atten = vec4(fogCol, fogAmt);
  #endif

  gl_Position = clip;
}
