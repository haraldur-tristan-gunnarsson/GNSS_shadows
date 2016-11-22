#version 130

out vec4 out_color;

uniform sampler2D shadow_sampler;

void main(){
	vec2 xy = gl_FragCoord.xy/1024;//XXX Will only work for 1024x1024 windows.
	out_color = vec4(texture(shadow_sampler, xy).x);
}
