#version 450

layout(location = 1) uniform vec4 in_colour;
out vec4 out_colour;

void main(){
	out_colour = in_colour;
}
