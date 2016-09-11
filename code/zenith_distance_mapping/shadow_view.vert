#version 130

in vec4 position;

uniform mat4 model_mat;
uniform mat4 shad_mat;

void main(){
   gl_Position = shad_mat * model_mat * position;
}
