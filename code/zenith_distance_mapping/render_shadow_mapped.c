#include <assert.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif//#ifndef M_PI

#include "linmath.h"
#include "chores.h"

static render_target shadow;
static unsigned int shadow_view = 0;//If 0, show shadows, else show depth map.
static GLint model_mat, proj_mat, shad_mat;
static unsigned int win_x, win_y;
static mesh model;
static mesh ground;

void initialize(const unsigned int arg_win_x, const unsigned int arg_win_y, const char *const *const argv, const int argc){
	static int called = 0;//Important not to cause GPU memory leaks.
	if(NULL == argv || 0 == argc)puts("Switched view.");
	GLuint shaderProgram;//Leaking memory, but not a lot (user mode switch is usually infrequent and the amount each time is small).
	if(shadow_view)shaderProgram = create_program("shadow_view.vert", "shadow_view.frag");
	else{
		shaderProgram = create_program("shadow_mapped.vert", "shadow_mapped.frag");
		proj_mat = glGetUniformLocation(shaderProgram, "proj_mat");
	}
	const GLint shadow_sampler = glGetUniformLocation(shaderProgram, "shadow_sampler");
	const GLint shadow_sampler_texture = 0;
	glUniform1i(shadow_sampler, shadow_sampler_texture);
	const GLuint position_attribute = glGetAttribLocation(shaderProgram, "position");//Get the location of the  position attribute that enters the vertex shader
	assert((GLint)position_attribute > -1);
	shad_mat = glGetUniformLocation(shaderProgram, "shad_mat");
	model_mat = glGetUniformLocation(shaderProgram, "model_mat");
	//printf("%s %d\n",__FILE__, __LINE__);
	if(!called){//Important not to cause GPU memory leaks.
		win_x = arg_win_x;
		win_y = arg_win_y;
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

		called = 1;//Important not to cause GPU memory leaks.
	}
}

void renderer_specific_commands(int key){
	switch(key){
		case 'S'://Switch views.
			shadow_view = shadow_view ? 0 : 1;
			initialize(win_x,win_y,NULL,0);
			break;
	}
}

inline static void draw_mesh(const mesh *item){
	glUniformMatrix4fv(model_mat, 1, GL_FALSE, *(item->model_mat));
	glBindVertexArray(item->vao);
	glDrawElements(GL_TRIANGLES, item->draw_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

inline static void render_shadow(void){
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadow.fbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	draw_mesh(&model);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

int display(void){
	static unsigned int iii = 0;

	//Creating an oblique projection (an orthographic projection that has been sheared):
	//float azimuth = (M_PI/180) * (iii % 360), zenith_distance = (M_PI/180) * ((iii/360) % 90);//Changes azimuth faster.
	float azimuth = (M_PI/180) * ((iii/90) % 360), zenith_distance = (M_PI/180) * (iii % 90);//Changes zenith_distance faster.
	float offs = tan(zenith_distance);//Shearing increases as zenith_distance increases.
	float xoffs = offs * sin(azimuth);
	float yoffs = offs * cos(azimuth);
	mat4x4 raw_shad_mat_mat = {//Column major
		{1.0f,  0.0f,  0.0f,  0.0f},
		{0.0f,  1.0f,  0.0f,  0.0f},
		{xoffs, yoffs, 1.0f,  0.0f},
		{0.0f,  0.0f,  0.0f,  1.0f},
	};
	mat4x4 shad_mat_mat, ortho_mat;
	mat4x4_ortho(ortho_mat, -1, 1, -1, 1, -model.zmin/512, -ground.zmax/512);//-1 to 1, not 0 to 1, so 512 rather than 1024.
	mat4x4_mul(shad_mat_mat, ortho_mat, raw_shad_mat_mat);

	glUniformMatrix4fv(shad_mat, 1, GL_FALSE, *shad_mat_mat);
	if(!shadow_view)glUniformMatrix4fv(proj_mat, 1, GL_FALSE, *shad_mat_mat);//Create depth map from satellite's point of view.
	render_shadow();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//Clear colour to keep background, clear depth to prevent artefacts in depth map mode.
	if(!shadow_view)glUniformMatrix4fv(proj_mat, 1, GL_FALSE, *ortho_mat);//Draw shadows from zenith view.
	draw_mesh(&ground);

	glfwSwapBuffers();
	iii++;
	return 0;//Continue forever, until program is terminated by the user.
}
