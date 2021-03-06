#include <assert.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif//#ifndef M_PI

#include "linmath.h"
#include "chores.h"
#include "dbg.h"
#include <stdlib.h>

void add_shader_to_program(const char *shader_path, const GLenum shader_type, const GLuint shader_program){
	GLuint shader = load_and_compile_shader(shader_path, shader_type);
	glAttachShader(shader_program, shader);
	glDeleteShader(shader);
}

static GLint satellite_vector;
static unsigned int win_x, win_y;
static mesh model;
static mesh ground;
static GLuint generate_shadowed;
static GLuint generate_shad_vols;
static mat4x4 orthographic_projection;

void initialize(const unsigned int arg_win_x, const unsigned int arg_win_y, const char *const *const argv, const int argc){
	//printf("%s %d\n",__FILE__, __LINE__);
	generate_shad_vols = glCreateProgram();
	add_shader_to_program("shadow_volume.vert", GL_VERTEX_SHADER, generate_shad_vols);
	add_shader_to_program("shadow_volume.geom", GL_GEOMETRY_SHADER, generate_shad_vols);
	add_shader_to_program("shadow_volume.frag", GL_FRAGMENT_SHADER, generate_shad_vols);
	glLinkProgram(generate_shad_vols);
	generate_shadowed = glCreateProgram();
	add_shader_to_program("basic.vert", GL_VERTEX_SHADER, generate_shadowed);
	add_shader_to_program("basic.frag", GL_FRAGMENT_SHADER, generate_shadowed);
	glLinkProgram(generate_shadowed);

	win_x = arg_win_x;
	win_y = arg_win_y;
	const GLuint position_attribute = 0;
	GLint proj_mat = 0;
	satellite_vector = 2;

	model = init_model(argv[1], position_attribute, win_x, win_y, 0, 0, 0);
	if(2 < argc)ground = init_model(argv[2],position_attribute, win_x, win_y, model.xmin, model.ymin, 1);
	else ground = model;

	mat4x4_ortho(orthographic_projection, -1, 1, -1, 1, -model.zmin/512, -ground.zmax/512);//-1 to 1, not 0 to 1, so 512 rather than 1024.
	glUseProgram(generate_shadowed);
	glUniformMatrix4fv(proj_mat, 1, GL_FALSE, *orthographic_projection);
	glUseProgram(generate_shad_vols);
	glUniformMatrix4fv(proj_mat, 1, GL_FALSE, *orthographic_projection);

	//Note that this program will effectively discard the lowest vertex of the ground model, so any ground planes must have at least one vertex below them (or a second plane) in order to display anything.
	glEnable(GL_DEPTH_TEST);//By default, behaves as glDepthFunc(GL_LESS);
	//glEnable(GL_DEPTH_CLAMP);//Using this makes the program 3x slower on my home machine (nVidia 750Ti Arch Linux)
	glEnable(GL_STENCIL_TEST);
	glClearStencil(0);//Already 0 by default, according to the standard, but it is set here just in case, as the code depends on this behaviour.
	glClearColor(0, 1, 0, 1);
}

void renderer_specific_commands(int key){(void)key;}//Nothing to do, here

inline static void draw_mesh(const mesh *item){
	glBindVertexArray(item->vao);
	glDrawElements(GL_TRIANGLES, item->draw_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

int display(void){
	static const GLuint draw_colour = 1;
	static const int stencil_ref_value = 1;

	static unsigned int iii = 0;//This procedure is like a loop (returning non-zero terminates), and this static variable is the loop counter.
	int azim_deg = iii / 90;
	int elev_deg = iii % 90;
	if(360 == azim_deg)return 1;//All azimuths generated.

	if(!elev_deg){//This part is executed first, and rarely; it excludes unavailable space (under buildings or outside the model area):
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);//disabled
		glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glUseProgram(generate_shadowed);

		//Exclude (with stencil) area outside ground model:
		glStencilMask(0xFF);//enabled
		glStencilFunc(GL_ALWAYS, stencil_ref_value, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);//should have the same effect as all GL_REPLACE, but may be a little faster depending on the OpenGL driver
		draw_mesh(&ground);

		//Exclude (with stencil) areas beneath buildings (also excludes areas beneath overhangs):
		glStencilFunc(GL_ALWAYS, 0, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		draw_mesh(&model);

		//Draw all non-excluded areas white and set depth buffer for shadows:
		glStencilMask(0x00);//disabled
		glStencilFunc(GL_EQUAL, stencil_ref_value, 0xFF);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);//enabled
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//Clear to background colour (green).
		glUniform4f(draw_colour, 1.0, 1.0, 1.0, 1.0);
		draw_mesh(&ground);
	}
	else{//This part is executed most of the time, but is not first:
		//Generate shadow vector used by geometry shader:
		static const double radian_ratio = M_PI/180;
		double azimuth   = radian_ratio * (azim_deg - 2.5);//True North is roughly 2.5 degrees west of Grid North in London.
		double elevation = radian_ratio * elev_deg;
		float horiz = sin(elevation);
		static const int scaler = 16;//Need not scale the vector by much, as the model is already scaled down proportional to win_x i.e. with win_x at 1024, vector is already 512 metres long.
		vec4 satellite_vector_RAW = {	-horiz*sin(azimuth)*scaler,
						-horiz*cos(azimuth)*scaler,
						cos(elevation)*scaler,
						0};//left-handed coordinate system, so Z reversed.
		vec4 satellite_vector_PROJECTED;
		mat4x4_mul_vec4(satellite_vector_PROJECTED, orthographic_projection, satellite_vector_RAW);

		//Record shadows (with incorrect overhangs) with shade derived from the zenith distance (elev_deg):
		glStencilMask(0xFF);//enabled
		glStencilFunc(GL_EQUAL, stencil_ref_value, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);//This does not treat overhangs correctly, but conversion to building boundaries discards overhang data anyway so no matter.
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);//enabled
		glUseProgram(generate_shad_vols);
		glUniform4fv(satellite_vector, 1, satellite_vector_PROJECTED);
		float shade = elev_deg * 1.0/255;
		glUniform4f(draw_colour, shade, shade, shade, 1.0);
		//glEnable(GL_CULL_FACE);//By default, cull back faces, which are clockwise by default (i.e. counter-clockwise is front by default). Does not work with the Fenchurch ZMapping model, but has a minor speed improvement (18 seconds down to 16 for 10 azimuths without saving to disk).
		draw_mesh(&model);//Record shadows (with overhangs incorrect)
		//glDisable(GL_CULL_FACE);

		//Reset the depth values to those from the ground model only (alternative: use glDepthMask above, though that may be slower overall):
		glStencilMask(0x00);//disabled
		glStencilFunc(GL_EQUAL, stencil_ref_value, 0xFF);
		glUseProgram(generate_shadowed);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);//disabled
		glClear(GL_DEPTH_BUFFER_BIT);
		draw_mesh(&ground);
	}

	glfwSwapBuffers();
	//if(89 == elev_deg)glfwSwapBuffers();
	if(89 == elev_deg){
		unsigned char *image = malloc(1024*1024*3*sizeof(unsigned char));
		glReadPixels(0,0, win_x,win_y, GL_RGB, GL_UNSIGNED_BYTE, image);
		//The centre of the screen is position (0,0,0). From chores.c (init_model), xmin is at -0.5x, ymin at -0.5y
		//Currently, the window is 1024 pixels square (TODO: replace magic number 1024, everywhere)
		//The centre (0,0,0) is BETWEEN the four centre pixels and (-1,-1,0) is half a pixel below-left of the bottom-left pixel.
		//Want to give the coordinate of the bottom-left PIXEL.
		//Therefore, pix_xmin = xmin - (1024/4) + 0.5 -- i.e. xmin - 255.5
		static const float xmin_offset = (1024/4) - 0.5;
		static const float ymin_offset = (1024/4) + 0.5;
		char filename[50];
		sprintf(filename, "%f_%f_%d.png", model.xmin - xmin_offset, model.ymin - ymin_offset, azim_deg);
		save_PNG(filename, image, 1024*1024*3*sizeof(unsigned char), win_x, win_y);
		free(image); image = NULL;
	}
	iii++;
	return 0;
}
