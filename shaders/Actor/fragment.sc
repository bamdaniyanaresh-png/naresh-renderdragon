$input v_vcol, v_lit, v_atten, v_uv

#include <bgfx_shader.sh>
#include <MinecraftRenderer.Materials/ActorUtil.dragonh>
#include <naresh.sh>

uniform vec4 ColorBased;
uniform vec4 ChangeColor;
uniform vec4 UseAlphaRewrite;
uniform vec4 TintedAlphaTestEnabled;
uniform vec4 MatColor;
uniform vec4 OverlayColor;
uniform vec4 MultiplicativeTintColor;
uniform vec4 ActorFPEpsilon;
uniform vec4 HudOpacity;

SAMPLER2D_AUTOREG(s_MatTexture);
SAMPLER2D_AUTOREG(s_MatTexture1);

void main() {
  #if defined(DEPTH_ONLY) || defined(INSTANCING)
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    return;
  #elif defined(DEPTH_ONLY_OPAQUE)
    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    return;
  #endif

  vec4 col = getActorAlbedoNoColorChange(v_uv, s_MatTexture, s_MatTexture1, MatColor);

  #ifdef ALPHA_TEST
    float alpha = mix(col.a, col.a*OverlayColor.a, TintedAlphaTestEnabled.x);
    if (shouldDiscard(col.rgb, alpha, ActorFPEpsilon.x)) {
      discard;
    }
  #endif

  #ifdef CHANGE_COLOR_MULTI
    col = applyMultiColorChange(col, ChangeColor.rgb, MultiplicativeTintColor.rgb);
  #elif defined(CHANGE_COLOR)
    col = applyColorChange(col, ChangeColor, col.a);
    col.a *= ChangeColor.a;
  #endif

  #ifdef ALPHA_TEST
    col.a = max(UseAlphaRewrite.r, col.a);
  #endif

  col.rgb *= mix(vec3(1.0, 1.0, 1.0), v_vcol.rgb, ColorBased.x);
  col = applyOverlayColor(col, OverlayColor);
  col.rgb = nrsToLinear(col.rgb);

  vec4 lit = v_lit;
  #if defined(EMISSIVE) || defined(EMISSIVE_ONLY)
    lit.rgb = max(lit.rgb, vec3_splat(4.0*(1.0 - col.a)));
  #endif
  col = applyLighting(col, lit);

  #ifdef TRANSPARENT
    col = applyHudOpacity(col, HudOpacity.x);
  #endif

  col.rgb = nrsEntityFinish(col.rgb, v_atten.rgb, v_atten.a);
  gl_FragColor = col;
}
