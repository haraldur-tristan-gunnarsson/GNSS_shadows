#version 450

layout(location = 0) in vec4 position;

layout(location = 0) uniform mat4 proj_mat;

void main(){
   gl_Position = proj_mat * position;
}
