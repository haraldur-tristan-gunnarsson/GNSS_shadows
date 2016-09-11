#include <stdlib.h>//malloc, free
#include <assert.h>

#include "linmath.h"
#include "chores.h"

static render_target height_map;
static GLint model_mat, proj_mat;
static unsigned int win_x, win_y;
static mesh ground;
//static GLint /*win_x_x2,*/ zmin, inverse_z_range;

void initialize(const unsigned int arg_win_x, const unsigned int arg_win_y, const char *const *const argv, const int argc){
    const GLuint shaderProgram = create_program("height_map.vert", "height_map.frag");
	const GLuint position_attribute = glGetAttribLocation(shaderProgram, "position");//Get the location of the  position attribute that enters the vertex shader
    assert((GLint)position_attribute > -1);
    proj_mat = glGetUniformLocation(shaderProgram, "proj_mat");
    model_mat = glGetUniformLocation(shaderProgram, "model_mat");
    //win_x_x2 = glGetUniformLocation(shaderProgram, "win_x_x2");
//    zmin = glGetUniformLocation(shaderProgram, "zmin");
//    inverse_z_range = glGetUniformLocation(shaderProgram, "inverse_z_range");
//    printf("%s %d\n",__FILE__, __LINE__);
    win_x = arg_win_x;
    win_y = arg_win_y;
    if(2 < argc)ground = init_model(argv[2],position_attribute, win_x, win_y, 0, 0, 0);
    else{
        puts("Missing ground model.\n");
        error_exit();
    }
    //glUniform1i(win_x_x2, win_x/2);
//    glUniform1f(zmin, ground.zmin * 2.0 / win_x);
//    glUniform1f(inverse_z_range, (win_x/2.0)/(ground.zmax - ground.zmin));
    //printf("%d %f %f",2*win_x, ground.zmin, 1.0/(ground.zmax - ground.zmin));
    //ground = init_lei_ground_plane(position_attribute, win_x);//TODO: temporary.

    height_map = init_shadow_fbo(win_x,win_y);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, height_map.texture);

//    glEnable(GL_CULL_FACE);//By default, cull back faces, which are clockwise by default (i.e. counter-clockwise is front by default).
    glEnable(GL_DEPTH_TEST);//By default, behaves as glDepthFunc(GL_LESS);
    glDepthFunc(GL_LEQUAL);//Allows ground_plane to be at z-value 1.0 exactly.
    //glEnable(GL_DEPTH_CLAMP);//Would be bad to clip away potential shadowers, and depth precision is really only needed near the ground.  //TODO
    glClearColor(0, 1, 0, 1);
    //glClearColor(0, 1, 1, 1);
}

void renderer_specific_commands(int key){(void)key;}//Nothing to do, here


inline static void draw_mesh(const mesh *item){
    glUniformMatrix4fv(model_mat, 1, GL_FALSE, *(item->model_mat));
    glBindVertexArray(item->vao);
    glDrawElements(GL_TRIANGLES, item->draw_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

inline static void blit_to_screen(const GLuint fbo, const unsigned int win_x, const unsigned int win_y){
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, win_x, win_y, 0, 0, win_x, win_y, GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glfwSwapBuffers();
}

inline static void save_to_image(const GLuint fbo, const unsigned int win_x, const unsigned int win_y, const float xmin, const float ymin, const float zmin, const float zmax){
    const int image_size = win_x*win_y*3*sizeof(unsigned char);
    unsigned char *image = malloc(image_size);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glReadPixels(0,0, win_x,win_y, GL_RGB, GL_UNSIGNED_BYTE, image);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
//    for(size_t ii = 0; ii < 1024*1024*3; ++ii)printf("%d: %d\n", (int)ii, (int)image[ii]);
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
    const float min_offset_x = (win_x/4.0) - 0.5;
    const float min_offset_y = (win_y/4.0) - 0.5;
    char filename[50];
    sprintf(filename, "height_map_%f_%f_%f_%f.png", xmin - min_offset_x, ymin - min_offset_y, zmin, zmax);
    save_PNG(filename, image, image_size, win_x, win_y);
    free(image); image = NULL;
}

int display(void){//TODO
    mat4x4 raw_shad_mat_mat = {//Column major
        {1.0f,  0.0f,  0.0f,  0.0f},
        {0.0f,  1.0f,  0.0f,  0.0f},
        {0.0f,  0.0f,  1.0f,  0.0f},
        {0.0f,  0.0f,  0.0f,  1.0f},
    };
    mat4x4 shad_mat_mat, ortho_mat;
    int window_scale = win_x / 2;
    mat4x4_ortho(ortho_mat, -1, 1, -1, 1, -ground.zmin/window_scale, -ground.zmax/window_scale);//-1 to 1, not 0 to 1, so window_scale rather than 1024.    //TODO XXX FIXME Question whether this is appropriate here!
    mat4x4_mul(shad_mat_mat, ortho_mat, raw_shad_mat_mat);
    //printf("%f %f", ground.zmin, ground.zmax);


    //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, height_map.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//FIXME: needed?
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, height_map.texture);
//    glUniformMatrix4fv(proj_mat, 1, GL_FALSE, *raw_shad_mat_mat);//TODO So... orthographic projection,
    //glUniformMatrix4fv(proj_mat, 1, GL_FALSE, *shad_mat_mat);//TODO So... orthographic projection,
    glUniformMatrix4fv(proj_mat, 1, GL_FALSE, *ortho_mat);// but is it identity or not? TODO

    draw_mesh(&ground);
    //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    //blit_to_screen(height_map.fbo, win_x, win_y);
    //save_to_image(height_map.fbo, win_x, win_y, ground.xmin, ground.ymin, ground.zmin, ground.zmax);//TODO
    //printf("%s %d\n",__FILE__, __LINE__);
    glfwSwapBuffers();
    return 0;
    return 1;//By returning 1, the loop in main.c, that calls this procedure, will break, so the height map shall only be generated once.
}
