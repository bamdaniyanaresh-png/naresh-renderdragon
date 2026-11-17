$input a_texcoord0, a_position
#ifdef INSTANCING
  $input i_data0, i_data1, i_data2, i_data3
#endif
$output v_uv, v_dir

#include <bgfx_shader.sh>

uniform vec4 ViewPositionAndTime;

void main() {
  #ifdef INSTANCING
    mat4 model = mtxFromCols(i_data0, i_data1, i_data2, i_data3);
  #else
    mat4 model = u_model[0];
  #endif

  vec3 pos = mul(model, vec4(a_position, 1.0)).xyz;

  // direction of the view for this corner, before the sky box turns (the tornado does not turn with it)
  v_dir = pos;

  // the sky box turns once per hour
  highp float angle = 0.00174532925*ViewPositionAndTime.w;
  float s = sin(angle);
  float c = cos(angle);
  pos.xz = vec2(c*pos.x - s*pos.z, s*pos.x + c*pos.z);

  v_uv = 2.0*a_texcoord0;
  gl_Position = mul(u_viewProj, vec4(pos, 1.0));
}
