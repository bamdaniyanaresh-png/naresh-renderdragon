$input a_color0, a_position
#ifdef INSTANCING
  $input i_data0, i_data1, i_data2, i_data3
#endif
$output v_color0

#include <bgfx_shader.sh>

// the clouds are drawn in the sky shader, so the cloud mesh is thrown away
void main() {
  v_color0 = vec4(0.0, 0.0, 0.0, 0.0);
  gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
}
