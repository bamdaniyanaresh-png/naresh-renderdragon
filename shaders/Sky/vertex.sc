#ifndef INSTANCING
  $input a_color0, a_position
  $output v_ray, v_sundir
#endif

#include <bgfx_shader.sh>

#ifndef INSTANCING
  #include <naresh.sh>

  uniform vec4 TimeOfDay;
  #if NRS_SKY_RAIN
    uniform vec4 FogAndDistanceControl;
  #endif
#endif

void main() {
  #ifndef INSTANCING
    // the sky covers the whole screen. The depth is the same for every vertex,
    // so the ray is one smooth function of the screen position (no cuts between triangles)
    vec4 pos = vec4(2.0*clamp(a_position.xz, -0.5, 0.5), 0.5, 1.0);

    v_ray = mul(u_invViewProj, pos).xyz;

    // direction of the sun is the same for every pixel, so it is worked out here
    // the last value is how much it rains
    v_sundir = vec4(nrsSunTrue(TimeOfDay.x), NRS_SKY_RAIN_AMOUNT);

    gl_Position = pos;
  #else
    gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
  #endif
}
