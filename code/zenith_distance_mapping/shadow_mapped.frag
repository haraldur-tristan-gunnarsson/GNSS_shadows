#version 130

in vec4 shad_pos;
out vec4 out_colour;

uniform sampler2D shadow_sampler;

void main(){
	vec2 xy = shad_pos.xy * 0.5 + vec2(0.5, 0.5);//Convert from -1 to 1 into 0 to 1.
	float z = shad_pos.z * 0.5 + 0.5;//...
	float depth = texture(shadow_sampler, xy).x;
	if(depth > (z - 0.001))out_colour = vec4(1.0);
	else out_colour = vec4(vec3(0.0),1.0);
}
