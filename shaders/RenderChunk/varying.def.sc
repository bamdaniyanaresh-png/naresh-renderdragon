vec4 a_color0     : COLOR0;
vec3 a_position   : POSITION;
vec2 a_texcoord0  : TEXCOORD0;
vec2 a_texcoord1  : TEXCOORD1;

vec4 i_data0      : TEXCOORD8;
vec4 i_data1      : TEXCOORD7;
vec4 i_data2      : TEXCOORD6;
vec4 i_data3      : TEXCOORD5;

vec4 v_tint       : COLOR0;
vec4 v_vcol       : COLOR1;
vec4 v_ambient    : COLOR2;
vec4 v_sun        : COLOR3;
centroid vec2 v_uv : TEXCOORD0;
vec2 v_light      : TEXCOORD1;
vec3 v_wpos       : TEXCOORD2;
vec4 v_fog        : TEXCOORD3;
