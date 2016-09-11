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
	const GLuint position_attribute = glGetAttribLocation(shaderProgram, "position");//Get the location of the  position attribute that enters the vertex shader
    assert((GLint)position_attribute > -1);
    shad_mat = glGetUniformLocation(shaderProgram, "shad_mat");
    model_mat = glGetUniformLocation(shaderProgram, "model_mat");
//    printf("%s %d\n",__FILE__, __LINE__);
    win_x = arg_win_x;
    win_y = arg_win_y;
    map_A = init_fbo(win_x, win_y, GL_COLOR_ATTACHMENT0, GL_RGB, GL_RGB);
    map_B = init_fbo(win_x, win_y, GL_COLOR_ATTACHMENT0, GL_RGB, GL_RGB);
    //map_A = init_fbo(win_x, win_y, GL_COLOR_ATTACHMENT0, GL_RED, GL_RED);
    //map_B = init_fbo(win_x, win_y, GL_COLOR_ATTACHMENT0, GL_RED, GL_RED);
    model = init_model(argv[1], position_attribute, win_x, win_y, 0, 0, 0);
    if(2 < argc)ground = init_model(argv[2],position_attribute, win_x, win_y, model.xmin, model.ymin, 1);
    else ground = model;
 //   assert(model.xmin == ground.xmin && model.ymin == ground.ymin);//TODO: replace this with one of Zed's bebug macros.
//    ground = init_lei_ground_plane(position_attribute, win_x);//TODO: temporary.

    shadow = init_shadow_fbo(win_x,win_y);
    glActiveTexture(GL_TEXTURE0 + shadow_sampler_texture);
    glBindTexture(GL_TEXTURE_2D, shadow.texture);

//    glEnable(GL_CULL_FACE);//By default, cull back faces, which are clockwise by default (i.e. counter-clockwise is front by default).
    glEnable(GL_DEPTH_TEST);//By default, behaves as glDepthFunc(GL_LESS);
    glDepthFunc(GL_LEQUAL);//Allows ground_plane to be at z-value 1.0 exactly.
    glEnable(GL_DEPTH_CLAMP);//Would be bad to clip away potential shadowers, and depth precision is really only needed near the ground.
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

inline static void render_shadow(void){
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadow.fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    draw_mesh(&model);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

inline static void new_azimuth_refresh(const render_target *const src){//Resets zenith distance values
    glClearColor(1, 1, 1, 1);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, src->fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//FIXME: needed?
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glClearColor(0, 1, 0, 1);
}

inline static void blit_to_screen(const render_target *const src, const unsigned int win_x, const unsigned int win_y){
    glBindFramebuffer(GL_READ_FRAMEBUFFER, src->fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, win_x, win_y, 0, 0, win_x, win_y, GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/, GL_NEAREST);
    //printf("%d\n",-1 + (iii/90) % 360);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glfwSwapBuffers();
}

inline static void save_to_image(const render_target *const src, const unsigned int win_x, const unsigned int win_y, const unsigned int azimuth, const float xmin, const float ymin){
    unsigned char *image = malloc(1024*1024*3*sizeof(unsigned char));
    glBindFramebuffer(GL_READ_FRAMEBUFFER, src->fbo);
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
    static const float xmin_offset = (1024/4) - 0.5;
    static const float ymin_offset = (1024/4) + 0.5;
    char filename[50];
    sprintf(filename, "%f_%f_%d.png", xmin - xmin_offset, ymin - ymin_offset, azimuth);
    save_PNG(filename, image, 1024*1024*3*sizeof(unsigned char), win_x, win_y);
    free(image); image = NULL;
}

#define R_ANGLE 90//Define number of divisions in a right angle HERE. Cannot meaningfully exceed 256: this code exists in shader (at time of typing): else out_colour = vec4(vec3(float(zen_dist)/255),1.0); -- also, the FBOs currently use GL_RGB format, which is normalised 8-bits per colour integers/floats.
int display(void){//TODO: Beware state
    static unsigned int iii = 0;
    static const unsigned int r_angle = R_ANGLE;//A right angle in whichever system
    static const unsigned int semi = R_ANGLE*2;//180 degrees
    static const unsigned int circle = R_ANGLE*4;//Azimuths all around
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
    double azimuth   = (M_PI/semi) * (azimuth_count - 2.5);//True North is roughly 2.5 degrees west of Grid North in London.
    double elevation = (M_PI/semi) * (iii % r_angle);
    if(0 == (iii % r_angle))blit_to_screen(src, win_x, win_y);//XXX FIXME TODO Makes the computer noticeably less responsive. Why? Without the if statement it is fine! What the fuck? Why is the computer more responsive when buffer swapping is more frequent? Make a toy test case later, perhaps submit to stackoverflow.com. It does, as expected, make processing slower...
    if(0 < iii && 0 == (iii % r_angle))save_to_image(src, win_x, win_y, azimuth_count ? azimuth_count - 1 : circle - 1, model.xmin, model.ymin);
    if(circle <= iii/r_angle)return 1;//Have generated all images, one for each azimuth.
    if(0 == (iii % r_angle))new_azimuth_refresh(src);
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
    glUniformMatrix4fv(proj_mat, 1, GL_FALSE, *shad_mat_mat);

    render_shadow();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst->fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//FIXME: needed?
    glActiveTexture(GL_TEXTURE0 + map_sampler_texture);
    glBindTexture(GL_TEXTURE_2D, src->texture);
    glUniformMatrix4fv(proj_mat, 1, GL_FALSE, *ortho_mat);
    glUniform1i(zen_dist, iii % r_angle);

    draw_mesh(&ground);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    /*if(iii <1)*/iii++;
    return 0;
}
