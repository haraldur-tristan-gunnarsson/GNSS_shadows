#version 130

out vec4 out_color;

uniform sampler2D shadow_sampler;
//uniform sampler2DShadow shadow_sampler;
//uniform sampler2DRectShadow shadow_sampler;

void main(){
    vec2 xy = gl_FragCoord.xy/1024;
    out_color = vec4(texture(shadow_sampler, xy).x);
}
