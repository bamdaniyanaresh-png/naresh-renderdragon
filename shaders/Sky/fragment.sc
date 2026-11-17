#ifndef INSTANCING
  $input v_ray, v_sundir
#endif

#include <bgfx_shader.sh>

#ifndef INSTANCING
  #include <naresh.sh>

  uniform vec4 ViewPositionAndTime;
#endif

void main() {
  #ifndef INSTANCING
    highp vec3 ray = normalize(v_ray);
    highp vec3 S = normalize(v_sundir.xyz);

    // sun, moon, northern lights, clouds and the colour of the air
    highp vec3 light = nrsSkyFull(ray, S, ViewPositionAndTime.w, 1.0, v_sundir.w);

    gl_FragColor = vec4(nrsTone(light), 1.0);
  #else
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
  #endif
}
