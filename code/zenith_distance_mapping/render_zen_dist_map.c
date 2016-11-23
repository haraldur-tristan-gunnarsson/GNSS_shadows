#include <stdlib.h>//malloc, free
#include <assert.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif//#ifndef M_PI

#include "linmath.h"
#include "chores.h"

static render_target shadow, map_A, map_B;
static GLint zen_dist;
static const GLint map_sampler_texture = 1;
static GLint model_mat, proj_mat, shad_mat;
static unsigned int win_x, win_y;
static mesh model;
static mesh ground;

void initialize(const unsigned int arg_win_x, const unsigned int arg_win_y, const char *const *const argv, const int argc){
	const GLuint shaderProgram = create_program("zen_dist_map.vert", "zen_dist_map.frag");
	const GLint map_sampler = glGetUniformLocation(shaderProgram, "map_sampler");
	glUniform1i(map_sampler, map_sampler_texture);
	zen_dist = glGetUniformLocation(shaderProgram, "zen_dist");
	proj_mat = glGetUniformLocation(shaderProgram, "proj_mat");
	const GLint shadow_sampler = glGetUniformLocation(shaderProgram, "shadow_sampler");
	const GLint shadow_sampler_texture = 0;
	glUniform1i(shadow_sampler, shadow_sampler_texture);
	const GLuint position_attribute = glGetAttribLocation(shaderProgram, "position");//Get the location of the position attribute that enters the vertex shader
	assert((GLint)position_attribute > -1);
	shad_mat = glGetUniformLocation(shaderProgram, "shad_mat");
	model_mat = glGetUniformLocation(shaderProgram, "model_mat");

	//printf("%s %d\n",__FILE__, __LINE__);
	win_x = arg_win_x;
	win_y = arg_win_y;

	map_A = init_fbo(win_x, win_y, GL_COLOR_ATTACHMENT0, GL_RGB, GL_RGB);
	map_B = init_fbo(win_x, win_y, GL_COLOR_ATTACHMENT0, GL_RGB, GL_RGB);
	//map_A = init_fbo(win_x, win_y, GL_COLOR_ATTACHMENT0, GL_RED, GL_RED);
	//map_B = init_fbo(win_x, win_y, GL_COLOR_ATTACHMENT0, GL_RED, GL_RED);

	model = init_model(argv[1], position_attribute, win_x, win_y, 0, 0, 0);
	if(2 < argc)ground = init_model(argv[2],position_attribute, win_x, win_y, model.xmin, model.ymin, 1);
	else ground = model;

	shadow = init_shadow_fbo(win_x,win_y);
	glActiveTexture(GL_TEXTURE0 + shadow_sampler_texture);
	glBindTexture(GL_TEXTURE_2D, shadow.texture);

	//No face culling, as the models cannot be trusted to have consistent winding order.
	glEnable(GL_DEPTH_TEST);//By default, behaves as glDepthFunc(GL_LESS);
	glDepthFunc(GL_LEQUAL);//Allows ground_plane to be at z-value 1.0 exactly.
	glEnable(GL_DEPTH_CLAMP);//Would be bad to clip away potential shadowers, and depth precision is really only needed near the ground.
	glClearColor(0, 1, 0, 1);
}

void renderer_specific_commands(int key){(void)key;}//Nothing to do, here


inline static void draw_mesh(const mesh *item){
	glUniformMatrix4fv(model_mat, 1, GL_FALSE, *(item->model_mat));
	glBindVertexArray(item->vao);
	glDrawElements(GL_TRIANGLES, item->draw_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

inline static void new_azimuth_refresh(const render_target *const src){//Resets zenith distance values
	glClearColor(1, 1, 1, 1);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, src->fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glClearColor(0, 1, 0, 1);
}

inline static void blit_to_screen(const render_target *const src, const unsigned int win_x, const unsigned int win_y){
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src->fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, win_x, win_y, 0, 0, win_x, win_y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glfwSwapBuffers();
}

inline static void save_to_image(const render_target *const src, const unsigned int win_x, const unsigned int win_y, const unsigned int azimuth, const float xmin, const float ymin){
	unsigned char *image = malloc(1024*1024*3*sizeof(unsigned char));
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src->fbo);
	glReadPixels(0,0, win_x,win_y, GL_RGB, GL_UNSIGNED_BYTE, image);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	//The centre of the screen is position (0,0,0)
	//In the model initialization procedure:
	//vertex_array[ii * 3 + 0] -= xmin;
	//vertex_array[ii * 3 + 0] /= win_x;
	//vertex_array[ii * 3 + 0] -= 0.25;
	//vertex_array[ii * 3 + 0] /= 0.5;
	//vertex_array[ii * 3 + 1] -= ymin;
	//vertex_array[ii * 3 + 1] /= win_x;
	//vertex_array[ii * 3 + 1] -= 0.25;
	//vertex_array[ii * 3 + 1] /= 0.5;
	//Therefore, xmin is at -0.5x, ymin at -0.5y
	//Currently, the window is 1024 pixels square (TODO: replace magic number 1024, everywhere)
	//The centre (0,0,0) is BETWEEN the four centre pixels and (-1,-1,0) is half a pixel below-left of the bottom-left pixel.
	//Want to give the coordinate of the bottom-left PIXEL.
	//Therefore, pix_xmin = xmin - (1024/4) + 0.5 -- i.e. xmin - 255.5
	static const float xmin_offset = (1024/4) - 0.5;
	static const float ymin_offset = (1024/4) + 0.5;
	char filename[50];
	sprintf(filename, "%f_%f_%d.png", xmin - xmin_offset, ymin - ymin_offset, azimuth);
	save_PNG(filename, image, 1024*1024*3*sizeof(unsigned char), win_x, win_y);
	free(image); image = NULL;
}

#define R_ANGLE 90//Define number of divisions in a right angle HERE. Cannot meaningfully exceed 256: this code exists in shader (at time of typing): else out_colour = vec4(vec3(float(zen_dist)/255),1.0); -- also, the FBOs currently use GL_RGB format, which is normalised 8-bits per colour integers/floats.
int display(void){//Note: Beware state
	static const unsigned int r_angle = R_ANGLE;//A right angle in whichever system
	static const unsigned int semi = R_ANGLE*2;//180 degrees
	static const unsigned int circle = R_ANGLE*4;//Azimuths all around

	static unsigned int iii = 0;//This procedure is like a loop (returning non-zero terminates), and this static variable is the loop counter.

	//map_A and map_B alternate being source and destination, with the source used to determine whether each pixel is already shadowed.
	const render_target *src;
	const render_target *dst;
	if(iii % 2){
		src = &map_B;
		dst = &map_A;
	}else{
		src = &map_A;
		dst = &map_B;
	}

	unsigned int azimuth_count = (iii/r_angle) % circle;
	unsigned int zendist_count = iii % r_angle;
	if(0 == zendist_count)blit_to_screen(src, win_x, win_y);//XXX FIXME TODO Makes the computer noticeably less responsive. Why? Without the if statement it is fine! What the fuck? Why is the computer more responsive when buffer swapping is more frequent? Make a toy test case later, perhaps submit to stackoverflow.com. It does, as expected, make processing slower...			(It appears that this does not occur on all computers with all drivers.)
	if(0 < iii && 0 == zendist_count)save_to_image(src, win_x, win_y, azimuth_count ? azimuth_count - 1 : circle - 1, model.xmin, model.ymin);
	if(circle <= iii/r_angle)return 1;//Have generated all images, one for each azimuth.
	if(0 == zendist_count)new_azimuth_refresh(src);

	//Create an oblique projection (an orthographic projection that has been sheared) based on azimuth and zenith_distance of satellite:
	double azimuth		= (M_PI/semi) * (azimuth_count - 2.5);//True North is roughly 2.5 degrees west of Grid North in London.
	double zenith_distance	= (M_PI/semi) * zendist_count;
	float offs = tan(zenith_distance);//Shearing increases as zenith_distance increases.
	float xoffs = offs * sin(azimuth);
	float yoffs = offs * cos(azimuth);
	mat4x4 shearing_matrix = {//Column major
		{1.0f,  0.0f,  0.0f,  0.0f},
		{0.0f,  1.0f,  0.0f,  0.0f},
		{xoffs, yoffs, 1.0f,  0.0f},
		{0.0f,  0.0f,  0.0f,  1.0f},
	};
	mat4x4 satellite_view_projection, orthographic_projection;
	mat4x4_ortho(orthographic_projection, -1, 1, -1, 1, -model.zmin/512, -ground.zmax/512);//-1 to 1, not 0 to 1, so 512 rather than 1024.
	mat4x4_mul(satellite_view_projection, orthographic_projection, shearing_matrix);

	//Render a depth map in the satellite's view, from the city model (model):
	glUniformMatrix4fv(shad_mat, 1, GL_FALSE, *satellite_view_projection);
	glUniformMatrix4fv(proj_mat, 1, GL_FALSE, *satellite_view_projection);//Create depth map from SATELLITE'S point of view.
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadow.fbo);
		glClear(GL_DEPTH_BUFFER_BIT);
		draw_mesh(&model);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	//Update zenith distances of first shadowing, from the zenith view, using the ground model (ground):
	glUniformMatrix4fv(proj_mat, 1, GL_FALSE, *orthographic_projection);
	glUniform1i(zen_dist, zendist_count);
	glActiveTexture(GL_TEXTURE0 + map_sampler_texture);
	glBindTexture(GL_TEXTURE_2D, src->texture);//Previously updated zenith distances indicate whether an area is already shadowed.
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst->fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		draw_mesh(&ground);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	iii++;
	return 0;
}
