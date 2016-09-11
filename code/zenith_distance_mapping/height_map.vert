#version 130

in vec4 position;

uniform mat4 model_mat;
uniform mat4 proj_mat;
//uniform int win_x_x2;
//uniform float zmin;
//uniform float inverse_z_range;

void main(){
   gl_Position = proj_mat * model_mat * position;
   //gl_Position.z = (gl_Position.z * win_x_x2 - zmin) * inverse_z_range;
}
