#version 130

out vec4 out_colour;

void main(){
	out_colour = vec4(1 - gl_FragCoord.z);
}
