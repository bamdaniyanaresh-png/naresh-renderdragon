$input a_indices, a_color0, a_normal, a_position, a_texcoord0
#ifdef INSTANCING
  $input i_data1, i_data2, i_data3
#endif
$output v_vcol, v_lit, v_atten, v_uv

#include <bgfx_shader.sh>
#include <MinecraftRenderer.Materials/DynamicUtil.dragonh>
#include <MinecraftRenderer.Materials/TAAUtil.dragonh>
#include <naresh.sh>

uniform vec4 TileLightColor;
uniform mat4 Bones[8];
uniform vec4 UVAnimation;
uniform vec4 TimeOfDay;
#if NRS_ENTITY_RAIN
  uniform vec4 FogAndDistanceControl;
#endif

void main() {
  mat4 world = mul(u_model[0], Bones[int(a_indices)]);

  vec3 wpos;
  #ifdef INSTANCING
    mat4 inst = mtxFromCols(i_data1, i_data2, i_data3, vec4(0.0, 0.0, 0.0, 1.0));
    wpos = instMul(inst, vec4(a_position, 1.0)).xyz;
  #else
    wpos = mul(world, vec4(a_position, 1.0)).xyz;
  #endif

  vec4 clip = jitterVertexPosition(wpos);

  #if !(defined(DEPTH_ONLY_OPAQUE) || defined(DEPTH_ONLY) || defined(INSTANCING))
    vec3 normal = normalize(mul(world, a_normal)).xyz;

    vec3 lit;
    vec3 fogCol;
    float fogAmt;
    nrsEntityVertex(TimeOfDay.x, NRS_ENTITY_RAIN_AMOUNT, wpos, normal, TileLightColor, lit, fogCol, fogAmt);

    v_vcol = a_color0;
    v_lit = vec4(lit, 1.0);
    v_atten = vec4(fogCol, fogAmt);
    v_uv = applyUvAnimation(a_texcoord0, UVAnimation);
  #endif

  gl_Position = clip;
}
