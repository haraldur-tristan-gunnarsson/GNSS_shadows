#version 130

in vec4 position;

uniform mat4 model_mat;
uniform mat4 proj_mat;

void main(){
	gl_Position = proj_mat * model_mat * position;
}
