$input a_position, a_color0, a_texcoord0, a_indices, a_normal
#ifdef INSTANCING
  $input i_data0, i_data1, i_data2
#endif
$output v_vcol, v_lit, v_atten, v_uv, v_glint

#include <bgfx_shader.sh>
#include <MinecraftRenderer.Materials/DynamicUtil.dragonh>
#include <MinecraftRenderer.Materials/TAAUtil.dragonh>
#include <MinecraftRenderer.Materials/GlintUtil.dragonh>
#include <naresh.sh>

uniform vec4 TileLightColor;
uniform vec4 UVAnimation;
uniform vec4 UVScale;
uniform mat4 Bones[8];
uniform vec4 TimeOfDay;
#if NRS_ENTITY_RAIN
  uniform vec4 FogAndDistanceControl;
#endif

void main() {
  mat4 world = mul(u_model[0], Bones[int(a_indices)]);

  vec3 wpos;
  #ifdef INSTANCING
    mat4 inst = mtxFromCols(i_data0, i_data1, i_data2, vec4(0.0, 0.0, 0.0, 1.0));
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

    // two glint layers that move and turn on their own
    vec4 glint;
    glint.xy = calculateLayerUV(a_texcoord0, UVAnimation.x, UVAnimation.z, UVScale.xy);
    glint.zw = calculateLayerUV(a_texcoord0, UVAnimation.y, UVAnimation.w, UVScale.xy);

    v_vcol = a_color0;
    v_lit = vec4(lit, 1.0);
    v_atten = vec4(fogCol, fogAmt);
    v_uv = a_texcoord0;
    v_glint = glint;
  #endif

  gl_Position = clip;
}
