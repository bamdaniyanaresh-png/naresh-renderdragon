$input a_color0, a_position
$output v_color0

#include <bgfx_shader.sh>

uniform vec4 StarsColor;

void main() {
  #ifndef INSTANCING
    vec3 wpos = mul(u_model[0], vec4(a_position, 1.0)).xyz;

    v_color0 = vec4(a_color0.rgb*StarsColor.rgb, a_color0.a);
    gl_Position = mul(u_viewProj, vec4(wpos, 1.0));
  #else
    gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
  #endif
}
