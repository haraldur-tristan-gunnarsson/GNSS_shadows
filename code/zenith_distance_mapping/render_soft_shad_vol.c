#include <assert.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif//#ifndef M_PI

#include "linmath.h"
#include "chores.h"

static render_target stencil;
static GLint model_mat, proj_mat, shad_mat;
static unsigned int win_x, win_y;
static mesh model;
static mesh ground;

render_target init_stencil_fbo(const unsigned int windowX, const unsigned int windowY){//From http://ogldev.atspace.co.uk. Remember to delete FBOs when finished with.
    render_target stencil = init_fbo(windowX, windowY, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,stencil.fbo);
    glDrawBuffer(GL_NONE);//Disable writes to the color buffer
    //printf("%s %d\n",__FILE__, __LINE__);
    //glReadBuffer(GL_NONE);//May cause problems if set so and using OpenGL 3.x
    //printf("%s %d\n",__FILE__, __LINE__);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    return stencil;
}

void initialize(const unsigned int arg_win_x, const unsigned int arg_win_y, const char *const *const argv, const int argc){
	if(NULL == argv || 0 == argc)puts("blah");
	GLuint shaderProgram;//Probably leaking XXX
	shaderProgram = create_program("soft_shad_vol.vert", "soft_shad_vol.frag");
	proj_mat = glGetUniformLocation(shaderProgram, "proj_mat");
	const GLint shadow_sampler = glGetUniformLocation(shaderProgram, "shadow_sampler");
	const GLint shadow_sampler_texture = 0;
	glUniform1i(shadow_sampler, shadow_sampler_texture);
	const GLuint position_attribute = glGetAttribLocation(shaderProgram, "position");//Get the location of the  position attribute that enters the vertex shader
	assert((GLint)position_attribute > -1);
	shad_mat = glGetUniformLocation(shaderProgram, "shad_mat");
	model_mat = glGetUniformLocation(shaderProgram, "model_mat");
	//printf("%s %d\n",__FILE__, __LINE__);
	win_x = arg_win_x;
	win_y = arg_win_y;
	model = init_model(argv[1], position_attribute, win_x, win_y, 0, 0, 0);
	if(2 < argc)ground = init_model(argv[2],position_attribute, win_x, win_y, model.xmin, model.ymin, 1);
	else ground = model;

	stencil = init_shadow_fbo(win_x,win_y);
	glActiveTexture(GL_TEXTURE0 + shadow_sampler_texture);
	glBindTexture(GL_TEXTURE_2D, stencil.texture);

	//glEnable(GL_CULL_FACE);//By default, cull back faces, which are clockwise by default (i.e. counter-clockwise is front by default).
	glEnable(GL_DEPTH_TEST);//By default, behaves as glDepthFunc(GL_LESS);
	glDepthFunc(GL_LEQUAL);//Allows ground_plane to be at z-value 1.0 exactly.
	glEnable(GL_DEPTH_CLAMP);//Would be bad to clip away potential shadowers, and depth precision is really only needed near the ground.
	glClearColor(0, 1, 0, 1);
	//glClearColor(0, 1, 1, 1);
}

void renderer_specific_commands(int key){
	(void)key;
}

inline static void draw_mesh(const mesh *item){
	glUniformMatrix4fv(model_mat, 1, GL_FALSE, *(item->model_mat));
	glBindVertexArray(item->vao);
	glDrawElements(GL_TRIANGLES, item->draw_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

inline static void render_shadow(void){
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, stencil.fbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	draw_mesh(&ground);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

int display(void){
	mat4x4 ortho_mat;
	mat4x4_ortho(ortho_mat, -1, 1, -1, 1, -model.zmin/512, -ground.zmax/512);//-1 to 1, not 0 to 1, so 512 rather than 1024.

	glUniformMatrix4fv(shad_mat, 1, GL_FALSE, *ortho_mat);
	glUniformMatrix4fv(proj_mat, 1, GL_FALSE, *ortho_mat);

	render_shadow();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//FIXME: needed?
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);//FIXME: needed?

	draw_mesh(&ground);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
	glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	//glDrawBuffer(GL_NONE);//Disable writes to the color buffer

	draw_mesh(&model);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	//draw_mesh(&model);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glfwSwapBuffers();

	glDisable(GL_STENCIL_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);//FIXME: needed?
	return 0;
}
