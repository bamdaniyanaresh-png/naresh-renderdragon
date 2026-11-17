#ifndef NRS_SETTINGS_H
#define NRS_SETTINGS_H

// Settings of the shader. Change a value and build the pack again.
// A line that starts with // is switched off, remove the // to switch it on.

// ---- colour ----
#define NRS_GAMMA 2.2
#define NRS_EXPOSURE 1.0

// tone mapping is a mix of four curves, the weights are added up so they do not have to sum to 1
#define NRS_TM_SOFT 0.50           // soft exponential curve, main look, deep shadows and a smooth shoulder
#define NRS_TM_FILMIC 0.25         // filmic curve, contrast
#define NRS_TM_ACES 0.15           // cinematic curve, bright highlights
#define NRS_TM_REINHARD 0.10       // lifted shadows
#define NRS_TM_CONTRAST 0.22       // S curve after the mix, 0 to 1
#define NRS_TM_SATURATION 1.12
#define NRS_TM_WARMTH 0.03         // red up and blue down

// ---- world light ----
#define NRS_SUN_POWER 3.4          // strength of the sun
#define NRS_MOON_POWER 0.34        // strength of the moon light
#define NRS_AMBIENT_POWER 1.15     // strength of the sky light
#define NRS_SUN_TINT vec3(1.04, 1.0, 0.94)
#define NRS_AMBIENT_TINT vec3(0.96, 1.0, 1.06)
#define NRS_SHADOW_TINT vec3(0.88, 0.95, 1.08)   // extra tint of the sky light inside shadows
#define NRS_SHADOW_SOFTNESS 1.0    // width of the block shadow edge in light levels
#define NRS_AO_POWER 1.2           // 1 is the vertex shade as it is, more makes corners darker
#define NRS_TORCH_POWER 3.4        // strength of torch, lantern and fire light
#define NRS_TORCH_COLOR vec3(1.0, 0.52, 0.20)

// ---- fog ----
#define NRS_FOG_RANGE 160.0        // distance in blocks where the fog is almost full

// ---- sky ----
#define NRS_SKY_SATURATION 1.1
#define NRS_SUN_SIZE 0.045
#define NRS_MOON_SIZE 0.075        // big moon, 0.0045 would be the real size
#define NRS_MOON_GLOW 1.0          // 0 switches the glow off
#define NRS_MOON_POWER_DISC 2.4    // brightness of the moon disc
#define NRS_CLOUDS 1               // 0 switches the clouds off
#define NRS_CLOUD_COVER 0.53       // lower is more clouds
#define NRS_CLOUD_SCALE 0.95       // lower is bigger clouds
#define NRS_CLOUD_SPEED vec2(0.02, 0.05)
#define NRS_CIRRUS 1               // thin high clouds
#define NRS_NIGHT_SKY 1            // purple band of dust and gas with stars, only at night
#define NRS_NIGHT_SKY_POWER 1.0
#define NRS_NIGHT_BAND_WIDTH 2.2   // thickness of the purple band, 1.0 is thin
#define NRS_STAR_SMALL 0.955       // star cutoff of the smallest layer, closer to 1.0 means fewer small stars
#define NRS_STAR_DIM_CUT 0.25      // stars that are weaker than this part of their brightness are removed
#define NRS_STARS 1                // own stars (the stars of the game are switched off)
#define NRS_AURORA 1               // northern lights at night
#define NRS_AURORA_POWER 1.0

// ---- rain (everything here works only while it rains) ----
#define NRS_RAIN 1
#define NRS_RAIN_FOG_CLEAR 0.50    // fog start (ratio of the view distance): at or above this it is dry
#define NRS_RAIN_FOG_WET 0.30      // at or below this the rain is full
#define NRS_ENTITY_RAIN 1          // mobs and items follow the rain light, set 0 if a build error mentions FogAndDistanceControl
#define NRS_SKY_RAIN 1             // the sky follows the rain, set 0 if a build error mentions FogAndDistanceControl
#define NRS_RAIN_DEBUG 0           // 1 paints the world with the fog values (red = start, green = end, blue = distance/512), for calibration
#define NRS_RAIN_TEST 0            // 1 forces rain all the time, to check the look
#define NRS_RAIN_SUN_HIDE 0.92     // how much the sun and the moon fade away
#define NRS_RAIN_DARKEN 0.20       // world light gets darker, 0 none, 1 black
#define NRS_RAIN_WET 1.0           // wet ground that reflects the sky, 0 switches it off
#define NRS_RAIN_RIPPLE 1          // rain drop rings on the ground and on water, 0 switches them off
#define NRS_RAIN_RIPPLE_STRENGTH 1.0
#define NRS_RAIN_RIPPLE_SCALE 3.0  // 1.5 big rings, 6.0 tiny rings
#define NRS_RAIN_RIPPLE_SPEED 0.3  // 0.1 slow, 0.8 fast
#define NRS_RAIN_RIPPLE_RADIUS 1   // 1 = 9 cells (fast), 2 = 25 cells (denser, slower)
#define NRS_RAIN_RIPPLE_POWER 0.6  // brightness of the rings

// ---- water ----
#define NRS_WATER_TYPE 2           // 1 flat water, 2 wavy water
#define NRS_WATER_CLARITY 0.28     // opacity of the water body, lower is clearer
#define NRS_WATER_WAVE_STRENGTH 2.5 // height of the moving waves, 0 is a mirror
#define NRS_WATER_WAVE_SPEED 1.0   // 0.3 calm, 2.0 turbulent

// ---- quality ----
#define NRS_CLOUD_OCTAVES 4.0
#define NRS_AURORA_STEPS 8.0

// ---- redstone helpers ----
#define NRS_LIGHT_MAP 0            // 1 shows the light level borders
#define NRS_CHUNK_BOUND 0          // 1 shows the chunk borders

// enchant glint strength
#define NRS_GLINT 1.0

// ---- the End ----
#define NRS_END_SKY_LIGHT 1.0      // strength of the End sky texture
#define NRS_END_SKY_TINT vec3(0.30, 0.58, 1.0)   // colour of the End sky, the brightness of the texture stays
#define NRS_END_SKY_TINT_AMOUNT 0.85             // 0 keeps the purple of the game, 1 is fully tinted
#define NRS_END_HAZE vec3(0.004, 0.010, 0.022)   // soft blue haze of the sky, a little lighter at the horizon

// a turning storm funnel over the main island of the End (world position and size in blocks)
#define NRS_TORNADO 1              // 0 switches it off
#define NRS_TORNADO_X 0.0          // position of the middle line
#define NRS_TORNADO_Z 0.0
#define NRS_TORNADO_BASE 40.0      // height of the bottom
#define NRS_TORNADO_TOP 300.0      // height of the top
#define NRS_TORNADO_R0 16.0        // radius at the bottom
#define NRS_TORNADO_R1 150.0       // radius at the top
#define NRS_TORNADO_SPEED 1.0      // turning speed
#define NRS_TORNADO_DENSITY 1.0    // how thick the clouds of the funnel are
#define NRS_TORNADO_GLOW 1.0       // glow at the top
#define NRS_TORNADO_RANGE 700.0    // it is not worked out when you are further away than this (saves time)
#define NRS_TORNADO_FADE 900.0     // far parts fade out over this distance
#define NRS_TORNADO_OUTER 1        // thin outer clouds, 0 saves time

// the quality subpacks switch these, see pack.toml
#ifdef QUALITY_LOW
  #define NRS_NO_REFLECTION        // no water, glass and block reflections
  #undef NRS_CLOUD_OCTAVES
  #define NRS_CLOUD_OCTAVES 3.0
  #undef NRS_AURORA_STEPS
  #define NRS_AURORA_STEPS 5.0
  #undef NRS_CIRRUS
  #define NRS_CIRRUS 0
  #undef NRS_TORNADO_OUTER
  #define NRS_TORNADO_OUTER 0
#endif

#ifdef QUALITY_ULTRA
  #undef NRS_CLOUD_OCTAVES
  #define NRS_CLOUD_OCTAVES 5.0
  #undef NRS_AURORA_STEPS
  #define NRS_AURORA_STEPS 10.0
#endif

#endif
