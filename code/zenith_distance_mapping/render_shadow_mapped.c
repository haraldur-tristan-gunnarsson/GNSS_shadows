#include <assert.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif//#ifndef M_PI

#include "linmath.h"
#include "chores.h"

static render_target shadow;
static unsigned int shadow_view = 0;
static GLint model_mat, proj_mat, shad_mat;
static unsigned int win_x, win_y;
static mesh model;
static mesh ground;

void initialize(const unsigned int arg_win_x, const unsigned int arg_win_y, const char *const *const argv, const int argc){
    static int called = 0;//Important not to cause GPU memory leaks.
    if(NULL == argv || 0 == argc)puts("blah");
    GLuint shaderProgram;//Probably leaking XXX
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

//        glEnable(GL_CULL_FACE);//By default, cull back faces, which are clockwise by default (i.e. counter-clockwise is front by default).
//	glFrontFace(GL_CCW);
        glEnable(GL_DEPTH_TEST);//By default, behaves as glDepthFunc(GL_LESS);
        glDepthFunc(GL_LEQUAL);//Allows ground_plane to be at z-value 1.0 exactly.
        glEnable(GL_DEPTH_CLAMP);//Would be bad to clip away potential shadowers, and depth precision is really only needed near the ground.
        glClearColor(0, 1, 0, 1);
        //glClearColor(0, 1, 1, 1);

        called = 1;//Important not to cause GPU memory leaks.
    }
}

void renderer_specific_commands(int key){
    switch(key){
        case 'S':
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
    //float azimuth = (M_PI/180) * (iii % 360), elevation = (M_PI/180) * ((iii/360) % 90);
    float azimuth = (M_PI/180) * ((iii/90) % 360), elevation = (M_PI/180) * (iii % 90);
    float offs = tan(elevation);
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
    //mat4x4_ortho(ortho_mat, -1, 1, -1, 1, -model.zmin/512, -model.zmax/512);//-1 to 1, not 0 to 1, so 512 rather than 1024.
    //mat4x4_ortho(ortho_mat, -1, 1, -1, 1, -model.zmin/1024, -model.zmax/1024);
    mat4x4_mul(shad_mat_mat, ortho_mat, raw_shad_mat_mat);
//    float invariant_plane_Z = model.zmax/1024;
//    float invariant_plane_Z = -0.125;//XXX FIXME TODO Fudge.
//    shad_mat_mat[3][0] = -invariant_plane_Z * shad_mat_mat[2][0];//Correct for shear along Z with equivalent shear along W for desired Z value of invariant plane.
//    shad_mat_mat[3][1] = -invariant_plane_Z * shad_mat_mat[2][1];//...
    //mat4x4_translate_in_place(shad_mat_mat,0,0,-1);//Eh? Same level as ground and all fine! -1 to +1! Would anything be gained by adjusting the clipping planes and model matrices/vertices for geometry?

    glUniformMatrix4fv(shad_mat, 1, GL_FALSE, *shad_mat_mat);
    if(!shadow_view)glUniformMatrix4fv(proj_mat, 1, GL_FALSE, *shad_mat_mat);

    render_shadow();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//FIXME: needed?
    if(!shadow_view)glUniformMatrix4fv(proj_mat, 1, GL_FALSE, *ortho_mat);

    draw_mesh(&ground);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glfwSwapBuffers();
    iii++;
    return 0;
}
