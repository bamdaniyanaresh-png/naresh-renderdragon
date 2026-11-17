$input a_color0, a_position, a_texcoord0
#ifdef INSTANCING
  $input i_data1, i_data2, i_data3
#endif
$output v_light, v_occlusion, v_uv, v_uv1

#include <bgfx_shader.sh>
#include <MinecraftRenderer.Materials/FogUtil.dragonh>
#include <naresh.sh>

uniform vec4 Dimensions;
uniform vec4 ViewPosition;
uniform vec4 UVOffsetAndScale;
uniform vec4 FogAndDistanceControl;
uniform vec4 Velocity;
uniform vec4 TimeOfDay;
uniform vec4 PositionBaseOffset;
uniform vec4 PositionForwardOffset;

void main() {
  vec2 uv = UVOffsetAndScale.xy + a_texcoord0*UVOffsetAndScale.zw;

  #ifndef NO_VARIETY
    // the red channel picks one of the sprites
    uv.x += a_color0.x*255.0*UVOffsetAndScale.z;
  #endif

  // particles are placed inside a repeating box around the player
  const vec3 box = vec3(30.0, 30.0, 30.0);
  vec3 base = a_position + PositionBaseOffset.xyz;
  vec3 pos = base - box*trunc(base/box);
  pos += PositionForwardOffset.xyz - 0.5*box;

  bool isRain = UVOffsetAndScale.w > 3.8*UVOffsetAndScale.z && Dimensions.x < 0.1;
  if (isRain) {
    pos.x -= pos.y*Velocity.x;
  }

  vec3 bottom = pos;
  vec3 top = bottom + Velocity.xyz*Dimensions.y;

  vec4 clipBottom = mul(u_modelViewProj, vec4(bottom, 1.0));
  vec4 clipTop = mul(u_modelViewProj, vec4(top, 1.0));

  vec2 up = clipTop.xy/clipTop.w - clipBottom.xy/clipBottom.w;
  vec2 right = normalize(vec2(-up.y, up.x));

  vec4 clip = mix(clipTop, clipBottom, a_texcoord0.y);
  clip.xy += (0.5 - a_texcoord0.x)*right*Dimensions.x;

  float fog = calculateFogIntensity(clip.z, FogAndDistanceControl.z, FogAndDistanceControl.x, FogAndDistanceControl.y);

  vec3 rainLight;
  nrsWeatherLight(TimeOfDay.x, nrsRainAmount(FogAndDistanceControl), rainLight);

  v_light = vec4(rainLight, fog);
  v_occlusion = vec3(0.5 + (pos.xz + ViewPosition.xz)/64.0, (pos.y + ViewPosition.y - 0.5)/255.0);
  v_uv = uv;
  v_uv1 = a_texcoord0;

  gl_Position = clip;
}
