#include <assert.h>

#include "linmath.h"
#include "chores.h"

static render_target height_map;
static GLint model_mat, proj_mat;
static unsigned int win_x, win_y;
static mesh ground;

void initialize(const unsigned int arg_win_x, const unsigned int arg_win_y, const char *const *const argv, const int argc){
	const GLuint shaderProgram = create_program("height_map.vert", "height_map.frag");
	const GLuint position_attribute = glGetAttribLocation(shaderProgram, "position");//Get the location of the  position attribute that enters the vertex shader
	assert((GLint)position_attribute > -1);
	proj_mat = glGetUniformLocation(shaderProgram, "proj_mat");
	model_mat = glGetUniformLocation(shaderProgram, "model_mat");
	win_x = arg_win_x;
	win_y = arg_win_y;
	ground = init_model(argv[1],position_attribute, win_x, win_y, 0, 0, 0);
	(void)argc;//Unused, remove compiler warning.

	height_map = init_shadow_fbo(win_x,win_y);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, height_map.texture);

	glEnable(GL_DEPTH_TEST);//By default, behaves as glDepthFunc(GL_LESS);
	glDepthFunc(GL_LEQUAL);//Allows a ground plane to be at z-value 1.0 exactly.
	glClearColor(0, 1, 0, 1);
}

void renderer_specific_commands(int key){(void)key;}//Nothing to do, here

inline static void draw_mesh(const mesh *item){
	glUniformMatrix4fv(model_mat, 1, GL_FALSE, *(item->model_mat));
	glBindVertexArray(item->vao);
	glDrawElements(GL_TRIANGLES, item->draw_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

int display(void){
	mat4x4 ortho_mat;
	int window_scale = win_x / 2;
	mat4x4_ortho(ortho_mat, -1, 1, -1, 1, -ground.zmin/window_scale, -ground.zmax/window_scale);//-1 to 1, not 0 to 1, so window_scale rather than win_x.

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//FIXME: needed?
	glUniformMatrix4fv(proj_mat, 1, GL_FALSE, *ortho_mat);

	draw_mesh(&ground);
	glfwSwapBuffers();
	return 0;//By returning 0, ensures that this procedure ('display') is invoked indefinitely. Otherwise, the loop in main.c, that calls this procedure, will break.
}
