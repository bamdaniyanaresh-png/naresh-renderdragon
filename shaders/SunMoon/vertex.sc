$input a_position, a_texcoord0
$output v_texcoord0

#include <bgfx_shader.sh>

// the sun and the moon are drawn in the sky shader, so this quad is thrown away
void main() {
  v_texcoord0 = a_texcoord0;
  gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
}
