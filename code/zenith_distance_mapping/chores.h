#ifndef CHORES_H
#define CHORES_H
//C11+
#include <GL/glew.h>
#include <GL/glfw.h>//NOTE ********GLEW********: From http://www.glfw.org/faq.html: 2.15 - Can I use GLEW with GLFW? Yes, as long as you include the GLEW header before the GLFW one. The GLEW header defines all the necessary magic macros to make sure the gl.h that GLFW attempts to include doesn’t interfere.
#include <stdio.h>
#include "linmath.h"//mat4x4 model_mat

void error_exit(void);//Consider making this a varargs procedure calling fprintf

GLuint init_buffer(const GLenum target, const GLvoid *data, const GLsizeiptr sizeof_data);

GLuint init_vbo(const GLfloat verts[], const GLsizeiptr sizeof_verts);

GLuint init_eab(const GLuint indices[], const GLsizeiptr sizeof_indices);

GLuint init_vao(const GLuint vbo, const GLuint eab, const GLuint attribute);

typedef struct{//Any independent polyhedron/polygon to draw
    GLuint vao;
    GLuint vbo;
    GLuint eab;
    GLsizei draw_count;
    mat4x4 model_mat;
    float xmin;
    float xmax;
    float ymin;
    float ymax;
    float zmin;
    float zmax;
} mesh;

mesh init_ground_plane(const GLuint position_attribute);

mesh init_lei_ground_plane(const GLuint position_attribute, int win_x);

mesh init_cube(const GLuint position_attribute);

mesh init_model(const char *const filepath, const GLuint position_attribute, const unsigned int win_x, const unsigned int win_y, float xoffset, float yoffset, const int useoffset);

typedef struct{
    GLuint fbo;
    GLuint texture;
} render_target;

render_target init_fbo(const unsigned int windowX, const unsigned int windowY, const GLenum attachment_point, const GLenum internal_format, const GLenum format);

render_target init_shadow_fbo(const unsigned int windowX, const unsigned int windowY);

void save_PPM(const char *const fname, const unsigned char *const data, const size_t data_size, const size_t width, const size_t height);

void save_PNG(const char *const fname, unsigned char *const data, const size_t data_size, const size_t width, const size_t height);

GLuint load_and_compile_shader(const char *fname, GLenum shaderType);

GLuint create_program(const char *path_vert_shader, const char *path_frag_shader);

#endif //#ifndef CHORES_H
