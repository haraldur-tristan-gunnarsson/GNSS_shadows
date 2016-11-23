#version 130

in vec4 position;
out vec4 shad_pos;

uniform mat4 model_mat;
uniform mat4 proj_mat;
uniform mat4 shad_mat;

void main(){
	gl_Position = proj_mat * model_mat * position;
	shad_pos = shad_mat * model_mat * position;//Used to get depth values out of stored depth map, in fragment shader, so must transform as if viewing from the satellite.
}
