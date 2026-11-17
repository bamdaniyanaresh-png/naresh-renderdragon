$input v_vcol, v_lit, v_atten, v_uv

#include <bgfx_shader.sh>
#include <MinecraftRenderer.Materials/ActorUtil.dragonh>
#include <naresh.sh>

uniform vec4 ChangeColor;
uniform vec4 OverlayColor;
uniform vec4 ColorBased;
uniform vec4 MatColor;
uniform vec4 MultiplicativeTintColor;

SAMPLER2D_AUTOREG(s_MatTexture);

void main() {
  #if defined(DEPTH_ONLY) || defined(INSTANCING)
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    return;
  #endif

  vec4 col = MatColor*texture2D(s_MatTexture, v_uv);

  #ifdef ALPHA_TEST
    if (col.a < 0.5) {
      discard;
    }
  #endif

  #ifdef MULTI_COLOR_TINT
    col = applyMultiColorChange(col, ChangeColor.rgb, MultiplicativeTintColor.rgb);
  #else
    col = applyColorChange(col, ChangeColor, col.a);
  #endif

  col.rgb *= mix(vec3(1.0, 1.0, 1.0), v_vcol.rgb, ColorBased.x);
  col = applyOverlayColor(col, OverlayColor);

  col.rgb = nrsToLinear(col.rgb)*v_lit.rgb;
  col.rgb = nrsEntityFinish(col.rgb, v_atten.rgb, v_atten.a);
  gl_FragColor = col;
}
