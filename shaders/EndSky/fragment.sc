#ifndef INSTANCING
  $input v_uv, v_dir
#endif

#include <bgfx_shader.sh>

#ifndef INSTANCING
  #include <naresh.sh>

  uniform vec4 ViewPositionAndTime;
  uniform vec4 CameraPosition;

  SAMPLER2D_AUTOREG(s_SkyTexture);
#endif

void main() {
  #ifndef INSTANCING
    vec3 tex = texture2D(s_SkyTexture, v_uv).rgb;
    highp vec3 dir = normalize(v_dir);

    // the sky texture is shown as light, with the cold blue colour of the End
    highp vec3 light = nrsEndSky(nrsToLinear(tex)*NRS_END_SKY_LIGHT, dir);

    // the storm funnel over the main island
    #if NRS_TORNADO
      light = nrsTornado(light, CameraPosition.xyz, dir, ViewPositionAndTime.w);
    #endif

    gl_FragColor = vec4(nrsTone(light), 1.0);
  #else
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
  #endif
}
