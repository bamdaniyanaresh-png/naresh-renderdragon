$input v_color0

#include <bgfx_shader.sh>
#include <naresh.sh>

void main() {
  #if NRS_STARS
    // the stars are drawn in the sky shader
    discard;
  #endif
  gl_FragColor = v_color0;
}
