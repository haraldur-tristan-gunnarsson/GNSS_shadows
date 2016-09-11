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
static mat4x4 ortho_mat;

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
	satellite_vector = 1;
	model = init_model(argv[1], position_attribute, win_x, win_y, 0, 0, 0);
	if(2 < argc)ground = init_model(argv[2],position_attribute, win_x, win_y, model.xmin, model.ymin, 1);
	else ground = model;
	mat4x4_ortho(ortho_mat, -1, 1, -1, 1, -model.zmin/512, -ground.zmax/512);//-1 to 1, not 0 to 1, so 512 rather than 1024.
	glUseProgram(generate_shadowed);
	glUniformMatrix4fv(proj_mat, 1, GL_FALSE, *ortho_mat);
	glUseProgram(generate_shad_vols);
	glUniformMatrix4fv(proj_mat, 1, GL_FALSE, *ortho_mat);

	glEnable(GL_DEPTH_TEST);//By default, behaves as glDepthFunc(GL_LESS);
	//glDepthFunc(GL_LEQUAL);//Allows ground_plane to be at z-value 1.0 exactly.		XXX
	glEnable(GL_DEPTH_CLAMP);//Would be bad to clip away potential shadowers, and depth precision is really only needed near the ground.
	glEnable(GL_STENCIL_TEST);
	glClearStencil(0);//My code depends on this being so.
	glClearColor(0, 1, 0, 1);
}

void renderer_specific_commands(int key){
	switch(key){
		case 'S':
			//initialize(win_x,win_y,NULL,0);
			break;
    }
}

inline static void draw_mesh(const mesh *item){
	glBindVertexArray(item->vao);
	glDrawElements(GL_TRIANGLES, item->draw_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

int display(void){
	static const GLuint draw_colour = 1;
	static const int stencil_ref_value = 1;//90 - elevation;
	static unsigned int iii = 0;
	//float azimuth = (M_PI/180) * (iii % 360), elevation = (M_PI/180) * ((iii/360) % 90);
	float azimuth = (M_PI/180) * ((iii/90) % 360), elevation = (M_PI/180) * (iii % 90);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);//disabled
	glStencilMask(0xFF);//enabled
	glStencilFunc(GL_ALWAYS, stencil_ref_value, 0xFF);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glUseProgram(generate_shadowed);
	draw_mesh(&ground);//Exclude (with stencil) area outside ground model

	glDepthMask(GL_FALSE);//disabled
	glUseProgram(generate_shadowed);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	draw_mesh(&model);//Exclude (with stencil) areas underneath buildings (also excludes areas beneath overhangs)
	glDepthMask(GL_TRUE);//enabled

	//glDepthMask(GL_FALSE);//disabled
	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
	//TODO could use elevation value as colour uniform for shader as well as stencil reference value, to draw only in regions equal to the elevation value
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);//This does not treat overhangs correctly. XXX but conversion to building boundaries discards overhang data anyway so no matter.
	//glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);//This does not treat overhangs correctly. XXX but conversion to building boundaries discards overhang data anyway so no matter.
	//glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);//This one should be better, if it works
	glUseProgram(generate_shad_vols);
	float horiz = sin(elevation);
	static const int scaler = 16;//Need not scale the vector by much, as the model is already scaled down proportional to win_x i.e. with win_x at 1024, vector is already 512 metres long.
	vec4 sat_vec_vec = {-horiz*sin(azimuth)*scaler, -horiz*cos(azimuth)*scaler, cos(elevation)*scaler, 0};//left-handed coordinate system, so Z reversed.
	//glUniform4fv(satellite_vector, 1, sat_vec_vec);
	vec4 sat_vec_fin;
	mat4x4_mul_vec4(sat_vec_fin, ortho_mat, sat_vec_vec);
	glUniform4fv(satellite_vector, 1, sat_vec_fin);
	//printf("%d %d %f %f %f %f %f %f\n", (iii / 90) % 360, iii % 90, azimuth, elevation, horiz, horiz*sin(azimuth), horiz*cos(azimuth), cos(elevation));
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);//enabled
	glEnable(GL_CULL_FACE);//By default, cull back faces, which are clockwise by default (i.e. counter-clockwise is front by default).
	//glCullFace(GL_FRONT);
	draw_mesh(&model);//Record shadows (with overhangs incorrect)
	glDisable(GL_CULL_FACE);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);//enabled
	glDepthMask(GL_TRUE);//enabled
	glStencilMask(0x00);//disabled
	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
	glClear(GL_DEPTH_BUFFER_BIT);
	glUseProgram(generate_shadowed);
	glUniform4f(draw_colour, 0.0, 0.0, 0.0, 1.0);
	draw_mesh(&ground);//Draw all areas of ground black.
	glStencilFunc(GL_EQUAL, stencil_ref_value, 0xFF);
	glClear(GL_DEPTH_BUFFER_BIT);
	glUniform4f(draw_colour, 1.0, 1.0, 1.0, 1.0);
	draw_mesh(&ground);//Draw all unshadowed areas (with overhangs incorrect) white.

	glfwSwapBuffers();
	iii++;
	//printf("%d\n",iii);
	return 0;
}
