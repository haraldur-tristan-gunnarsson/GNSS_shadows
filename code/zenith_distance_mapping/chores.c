#include <float.h>
#include <stdlib.h>//atoi, malloc, free etc.
#include <png.h>
#include "dbg.h"
#include "linmath.h"
#include "chores.h"

void error_exit(void){//Consider making this a varargs procedure calling fprintf
    glfwTerminate();
    exit(EXIT_FAILURE);
}

GLuint init_buffer(const GLenum target, const GLvoid *data, const GLsizeiptr sizeof_data){//Remember to delete buffers later.
    GLuint handle;
    glGenBuffers(1, &handle);
    glBindBuffer(target, handle);
    glBufferData(target, sizeof_data, data, GL_STATIC_DRAW);
    glBindBuffer(target, 0);//Unbind.
    return handle;
}

GLuint init_vbo(const GLfloat verts[], const GLsizeiptr sizeof_verts){
    return init_buffer(GL_ARRAY_BUFFER, verts, sizeof_verts);
}

GLuint init_eab(const GLuint indices[], const GLsizeiptr sizeof_indices){
    return init_buffer(GL_ELEMENT_ARRAY_BUFFER, indices, sizeof_indices);
}

GLuint init_vao(const GLuint vbo, const GLuint eab, const GLuint attribute){//Remember to delete vaos later.
    GLuint handle;
	glGenVertexArrays(1, &handle);
	glBindVertexArray(handle);

	glEnableVertexAttribArray(attribute);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);//Specify how the data for attribute can be accessed
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eab);

	glBindVertexArray(0);
	glDisableVertexAttribArray(attribute);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return handle;
}

mesh init_lei_ground_plane(const GLuint position_attribute, int win_x){
    GLfloat ground_plane_verts[] = {
        -1,-1, -(17.0/win_x)/0.5,
         1,-1, -(17.0/win_x)/0.5,
         1, 1, -(17.0/win_x)/0.5,
        -1, 1, -(17.0/win_x)/0.5,
    };
    mesh ground_plane;
    ground_plane.vbo = init_vbo(ground_plane_verts, sizeof(ground_plane_verts));
    GLuint ground_plane_indices[] = {
        0, 1, 2,
        2, 3, 0,
    };
    ground_plane.draw_count = sizeof(ground_plane_indices)/sizeof(GLuint);
    ground_plane.eab = init_eab(ground_plane_indices, sizeof(ground_plane_indices));

    mat4x4_identity(ground_plane.model_mat);
    ground_plane.vao = init_vao(ground_plane.vbo, ground_plane.eab, position_attribute);
//TODO need zmin and zmax, for x and y too
    return ground_plane;
}

mesh init_ground_plane(const GLuint position_attribute){
    GLfloat ground_plane_verts[] = {
        -1,-1, 0,
         1,-1, 0,
         1, 1, 0,
        -1, 1, 0,
    };
    mesh ground_plane;
    ground_plane.vbo = init_vbo(ground_plane_verts, sizeof(ground_plane_verts));
    GLuint ground_plane_indices[] = {
        0, 1, 2,
        2, 3, 0,
    };
    ground_plane.draw_count = sizeof(ground_plane_indices)/sizeof(GLuint);
    ground_plane.eab = init_eab(ground_plane_indices, sizeof(ground_plane_indices));

    mat4x4_identity(ground_plane.model_mat);
    ground_plane.vao = init_vao(ground_plane.vbo, ground_plane.eab, position_attribute);
//TODO need zmin and zmax, for x and y too
    return ground_plane;
}

mesh init_cube(const GLuint position_attribute){
    GLfloat cube_verts[] = {
        -0.5,-0.5, 0.5,
         0.5,-0.5, 0.5,
         0.5, 0.5, 0.5,
        -0.5, 0.5, 0.5,
        -0.5,-0.5,-0.5,
         0.5,-0.5,-0.5,
         0.5, 0.5,-0.5,
        -0.5, 0.5,-0.5,
//        -0.5,-0.5, 0.5,
//         0.5,-0.5, 0.5,
//         0.5, 0.5, 0.5,
//        -0.5, 0.5, 0.5,
//        -0.5,-0.5,-0.25,
//         0.5,-0.5,-0.5,
//         0.5, 0.5,-0.0.525,
//        -0.5, 0.5,-0.5,

        -1,-1, 0,//ground-plane
         1,-1, 0,
         1, 1, 0,
        -1, 1, 0,
    };
    mesh cube;
    cube.vbo = init_vbo(cube_verts, sizeof(cube_verts));
    GLuint cube_indices[] = {//Clockwise from the outside, just in case
        0, 1, 2,//in, z=0.5
        2, 3, 0,

        6, 5, 4,//out, z=-0.5
        4, 7, 6,

        2, 1, 5,//right, x=0.5
        5, 6, 2,

        4, 0, 3,//left, x=-0.5
        3, 7, 4,

        7, 3, 2,//up, y=0.5
        2, 6, 7,

        1, 0, 4,//down, y=-0.5
        4, 5, 1,

        8, 9, 10,//ground-plane
        10, 11, 8,
    };
    cube.draw_count = sizeof(cube_indices)/sizeof(GLuint);
    cube.eab = init_eab(cube_indices, sizeof(cube_indices));
    mat4x4_identity(cube.model_mat);
    cube.vao = init_vao(cube.vbo, cube.eab, position_attribute);
    cube.zmin = -1;
    cube.zmax = 1;
    return cube;
}

mesh init_model(const char *const filepath, const GLuint position_attribute, const unsigned int win_x, const unsigned int win_y, float xoffset, float yoffset, const int useoffset){//TODO FIXME Needs cleanup. An excuse to learn how to use the preprocessor?
    //FILE *obj_file = stdin;
    //if('-' != filepath[0])obj_file = fopen(filepath, "r");
    FILE *obj_file = fopen(filepath, "r");
    check(obj_file, "Failed to open %s.", filepath);
    char line[256];
    unsigned int num_vertices = 0, num_conv_faces = 0;
    errno = 0;//For sensible errors, assuming any "check"s are failed.
    for(unsigned int linecount = 0; fgets(line, sizeof line, obj_file); ++linecount){
        check(!line[sizeof line - 1], "Line too long (more than %lu characters) in line %u of %s.", sizeof line, linecount, filepath);
        float x,y,z;
        int num_got = sscanf(line, "v %f %f %f", &x, &y, &z);
        check(3 == num_got || 0 == num_got, "Invalid vertex on line %u of %s.", linecount, filepath);
        if(3 == num_got)num_vertices++;
    }
    rewind(obj_file);
    for(unsigned int linecount = 0; fgets(line, sizeof line, obj_file); ++linecount){
        check(!line[sizeof line - 1], "Line too long (more than %lu characters) in line %u of %s.", sizeof line, linecount, filepath);
        if('f' == line[0]){
            unsigned int num_vert_indices = 0;
            for(char *marker = line + 1; '\n' != *marker && '\0' != *marker;){
                marker += strspn(marker, " \t");
                check(atoi(marker)/*atoi ignores initial whitespace*/, "Invalid face at line %u of %s.", linecount, filepath);//Wavefront .obj files are indexed from 1, so, clearly, any output from atoi that is zero indicates an error
                num_vert_indices++;
                marker += strcspn(marker, " \t");//Skip any '/'s and any texture or normal indices
                marker += strspn(marker, " \t");//Should reach the end of the line and so the \n or \0
            }
            check(3 == num_vert_indices || 4 == num_vert_indices, "Only triangles and quadrilaterals accepted, found unacceptable face at line %u of %s.", linecount, filepath);
            num_conv_faces++;
            if(4 == num_vert_indices)num_conv_faces++;
        }
    }
    printf("%u\n", num_vertices);
    printf("%u\n", num_conv_faces);
    unsigned int num_added_vertices = 0, num_added_faces = 0;
    GLfloat *vertex_array = malloc(sizeof(GLfloat) * 3 * num_vertices);
    GLuint *element_array = malloc(sizeof(GLuint) * 3 * num_conv_faces);
    float xmin = FLT_MAX;
    float ymin = FLT_MAX;
    float zmin = FLT_MAX;
    float xmax = -FLT_MAX;//FLT_MIN;     NOT FLT_MIN! FLT_MIN is the smallest POSITIVE
    float ymax = -FLT_MAX;//FLT_MIN;     normalized number representable with single
    float zmax = -FLT_MAX;//FLT_MIN;     precision float; use -FLT_MAX instead.
    rewind(obj_file);
    for(unsigned int linecount = 0; fgets(line, sizeof line, obj_file); ++linecount){
        check(!line[sizeof line - 1], "Line too long (more than %lu characters) in line %u of %s.", sizeof line, linecount, filepath);
        int num_got = sscanf(line, "v %f %f %f", &vertex_array[num_added_vertices * 3 + 0], &vertex_array[num_added_vertices * 3 + 1], &vertex_array[num_added_vertices * 3 + 2]);
        check(3 == num_got || 0 == num_got, "Invalid vertex on line %u of %s.", linecount, filepath);
        if(3 == num_got){
            vertex_array[num_added_vertices * 3 + 2] *= -1;
            xmin = xmin > vertex_array[num_added_vertices * 3 + 0] ? vertex_array[num_added_vertices * 3 + 0] : xmin;
            ymin = ymin > vertex_array[num_added_vertices * 3 + 1] ? vertex_array[num_added_vertices * 3 + 1] : ymin;
            zmin = zmin > vertex_array[num_added_vertices * 3 + 2] ? vertex_array[num_added_vertices * 3 + 2] : zmin;
            xmax = xmax < vertex_array[num_added_vertices * 3 + 0] ? vertex_array[num_added_vertices * 3 + 0] : xmax;
            ymax = ymax < vertex_array[num_added_vertices * 3 + 1] ? vertex_array[num_added_vertices * 3 + 1] : ymax;
            zmax = zmax < vertex_array[num_added_vertices * 3 + 2] ? vertex_array[num_added_vertices * 3 + 2] : zmax;
            num_added_vertices++;
        }
    }
    //XXX TODO: meditate on the purpose of xmin and determine if the following is appropriate:
    xmin = (int)xmin + 0.5;//Align whole-number coordinates to grid.
    ymin = (int)ymin - 0.5;//Align whole-number coordinates to grid.
    if(!useoffset){
        xoffset = xmin;
        yoffset = ymin;
    }
    for(size_t ii = 0; ii < num_added_vertices; ++ii){//The centre of the screen is position (0,0,0)
        vertex_array[ii * 3 + 0] -= xoffset;
        vertex_array[ii * 3 + 0] /= win_x;	//point at xmin now at 0 (centre)
        vertex_array[ii * 3 + 0] -= 0.25;	//now at -0.25 (off-centre) -- perhaps later this should be 0.5 so as to fit larger models with fewer wasted pixels.
        vertex_array[ii * 3 + 0] /= 0.5;	//double dimensions as 1 - -1 == 2
        vertex_array[ii * 3 + 1] -= yoffset;	//point at ymin now at 0 (centre)
        vertex_array[ii * 3 + 1] /= win_x;	//now at -0.25 (off-centre) -- perhaps later this should be 0.5 so as to fit larger models with fewer wasted pixels.
        vertex_array[ii * 3 + 1] -= 0.25;	//double dimensions as 1 - -1 == 2
        vertex_array[ii * 3 + 1] /= 0.5;
//        vertex_array[ii * 3 + 2] -= zmin;
        vertex_array[ii * 3 + 2] /= win_x;
        vertex_array[ii * 3 + 2] /= 0.5;
//        vertex_array[ii + 2] -= 0.5;
    }
    printf("%f %f %f\n",xmin,xmax,xmax - xmin);
    printf("%f %f %f\n",ymin,ymax,ymax - ymin);
    printf("%f %f %f\n",zmin,zmax,zmax - zmin);
    rewind(obj_file);
    for(unsigned int linecount = 0; fgets(line, sizeof line, obj_file); ++linecount){
        check(!line[sizeof line - 1], "Line too long (more than %lu characters) in line %u of %s.", sizeof line, linecount, filepath);
        if('f' == line[0]){
            unsigned int num_vert_indices = 0;
            int vert_indices[4];
            for(char *marker = line + 1; '\n' != *marker && '\0' != *marker;){
                marker += strspn(marker, " \t");
                vert_indices[num_vert_indices] = atoi(marker);//atoi ignores initial whitespace
                check(vert_indices[num_vert_indices], "Invalid face at line %u of %s.", linecount, filepath);//Wavefront .obj files are indexed from 1, so, clearly, any output from atoi that is zero indicates an error
                num_vert_indices++;
                marker += strcspn(marker, " \t");//Skip any '/'s and any texture or normal indices
                marker += strspn(marker, " \t");//Should reach the end of the line and so the \n or \0
            }
            check(3 == num_vert_indices || 4 == num_vert_indices, "Only triangles and quadrilaterals accepted, found unacceptable face at line %u of %s.", linecount, filepath);
            element_array[num_added_faces*3 + 0] = vert_indices[0] -1;//Wavefront .obj files index vertices from 1.
            element_array[num_added_faces*3 + 1] = vert_indices[1] -1;//Wavefront .obj files index vertices from 1.
            element_array[num_added_faces*3 + 2] = vert_indices[2] -1;//Wavefront .obj files index vertices from 1.
            num_added_faces++;
            if(4 == num_vert_indices){
                element_array[num_added_faces*3 + 0] = vert_indices[0] -1;//Wavefront .obj files index vertices from 1.
                element_array[num_added_faces*3 + 1] = vert_indices[2] -1;//Wavefront .obj files index vertices from 1.
                element_array[num_added_faces*3 + 2] = vert_indices[3] -1;//Wavefront .obj files index vertices from 1.
                num_added_faces++;
            }
//            printf("%d %d %d %d %u\n", vert_indices[0], vert_indices[1], vert_indices[2], vert_indices[3], num_vert_indices);
        }
    }
//    for(size_t ii = 0; ii < num_added_faces; ++ii){
//        printf("%d %d %d\n", element_array[ii*3 + 0], element_array[ii*3 + 1], element_array[ii*3 + 2]);
//        printf("%f %f %f\n", vertex_array[element_array[ii*3 + 0]], vertex_array[element_array[ii*3 + 1]], vertex_array[element_array[ii*3 + 2]]);
//    }
//    for(size_t ii = 0; ii < 100; ++ii){
//        printf("%f %f %f\n", temp[ii*3 + 0], temp[ii*3 + 1], temp[ii*3 + 2]);
//        printf("%f %f %f\n", temp_2[ii*3 + 0], temp_2[ii*3 + 1], temp_2[ii*3 + 2]);
//        printf("%f %f %f\n", vertex_array[ii*3 + 0], vertex_array[ii*3 + 1], vertex_array[ii*3 + 2]);
//    }
    mesh model;
    model.vbo = init_vbo(vertex_array, sizeof(GLfloat) * 3 * num_vertices);
    model.draw_count = num_added_faces * 3;
    model.eab = init_eab(element_array, sizeof(GLuint) * model.draw_count);
    mat4x4_identity(model.model_mat);
    (void)win_y;
    model.vao = init_vao(model.vbo, model.eab, position_attribute);
    model.xmin = xmin;
    model.xmax = xmax;
    model.ymin = ymin;
    model.ymax = ymax;
    model.zmin = zmin;
    model.zmax = zmax;
    free(vertex_array); vertex_array = NULL;
    free(element_array); element_array = NULL;
    check(!fclose(obj_file), "Failed to close %s.", filepath);
    return model;
    goto error;
error:
    if(obj_file)fclose(obj_file);
    free(vertex_array); vertex_array = NULL;
    free(element_array); element_array = NULL;
    error_exit();
    return init_cube(position_attribute);//Consider displaying a cube and ground-plane in the event of failure.
}

render_target init_fbo(const unsigned int windowX, const unsigned int windowY, const GLenum attachment_point, const GLenum internal_format, const GLenum format){//Derived from http://ogldev.atspace.co.uk. Remember to delete FBOs when finished with.
    GLuint fbo, texture;
    glGenFramebuffers(1, &fbo);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, windowX, windowY, 0, format, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//GL_LINEAR);//GL_NEAREST?
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//GL_LINEAR);//GL_NEAREST?
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);//Better to see a shadow abruptly stop than to see it repeat: more obvious
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);//...

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);//GL_DRAW_FRAMEBUFFER or GL_FRAMEBUFFER?
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment_point, GL_TEXTURE_2D, texture, 0);//Possible nVidia problem here? https://www.opengl.org/wiki/Common_Mistakes

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Status);
        error_exit();
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    render_target r_t;
    r_t.fbo = fbo;
    r_t.texture = texture;
    return r_t;
}

render_target init_shadow_fbo(const unsigned int windowX, const unsigned int windowY){//From http://ogldev.atspace.co.uk. Remember to delete FBOs when finished with.
    render_target shadow = init_fbo(windowX, windowY, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,shadow.fbo);
    glDrawBuffer(GL_NONE);//Disable writes to the color buffer
    //printf("%s %d\n",__FILE__, __LINE__);
    //glReadBuffer(GL_NONE);//May cause problems if set so and using OpenGL 3.x
    //printf("%s %d\n",__FILE__, __LINE__);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    return shadow;
}

void save_PPM(const char *const fname, const unsigned char *const data, const size_t data_size, const size_t width, const size_t height){
    check(width*height*3 <= data_size, "Image dimensions %d %d larger than data supplied %d", (int)width, (int)height, (int)data_size);
    check(width > 0 && height > 0, "Malformed image width %d height %d", (int)width, (int)height);
    FILE *out = fopen(fname, "w");
    fprintf(out, "P3\n%d %d 255\n", (int)width, (int)height);
    for(size_t jj = height; jj != 0; --jj){//Data received from glReadPixels has Y axis pointing up.
        for(size_t ii = 0; ii < width; ++ii){
            fprintf(out, "%3d %3d %3d\n"
                    ,(int)data[(jj-1)*width*3 + ii*3 + 0]
                    ,(int)data[(jj-1)*width*3 + ii*3 + 1]
                    ,(int)data[(jj-1)*width*3 + ii*3 + 2]
                   );
        }
    }
    fclose(out);
    return;
error:
    error_exit();
}

void save_PNG(const char *const fname, unsigned char *const data, const size_t data_size, const size_t width, const size_t height){
    check(width*height*3 <= data_size, "Image dimensions %d %d larger than data supplied %d", (int)width, (int)height, (int)data_size);
    check(width > 0 && height > 0, "Malformed image width %d height %d", (int)width, (int)height);

    FILE *out = NULL;
    check(out = fopen(fname, "wb"), "Failed to open %s\n", fname);
    png_structp out_write_struct = NULL;
    check(out_write_struct = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL), "Failed write struct allocation for %s\n", fname);
    png_infop out_info_struct = NULL;
    check(out_info_struct = png_create_info_struct(out_write_struct), "Failed info struct allocation for %s\n", fname);
    check(!setjmp(png_jmpbuf(out_write_struct)), "PNG creation error -- setjmp exception -- for %s\n", fname);//libpng uses longjmp for exception handling

    png_init_io(out_write_struct, out);
    png_set_IHDR(out_write_struct, out_info_struct, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(out_write_struct, out_info_struct);

    int row_stride = width*3;
    for(int jj = data_size - row_stride; jj >= 0; jj -= row_stride){//Data received from glReadPixels has Y axis pointing up.
        png_write_row(out_write_struct, jj + data);
    }
    png_write_end(out_write_struct, NULL);

    fclose(out);
    png_free_data(out_write_struct, out_info_struct, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&out_write_struct, NULL);
    return;
error:
    if(out)fclose(out);
    if(out_info_struct)png_free_data(out_write_struct, out_info_struct, PNG_FREE_ALL, -1);
    if(out_write_struct)png_destroy_write_struct(&out_write_struct, NULL);
    error_exit();
}

GLuint load_and_compile_shader(const char *fname, GLenum shaderType){
    errno = 0;
    GLchar *src = NULL;
    FILE *in = fopen(fname,"r");
    check(in, "Failed to open %s.", fname);

    check(!fseek(in,0,SEEK_END), "Seek error %s.", fname);//Get the number of bytes stored in this file
    size_t length = (size_t)ftell(in);//...
    rewind(in);//...
    src = malloc(sizeof(GLchar)*(length+1));
    check_mem(src);
    size_t length_read = fread(src,sizeof(char),length,in);
    check(length_read == length, "Amount read matches not the file length of %s.", fname);
    src[length] = '\0';//Add a valid C - string end, just in case

    check(!fclose(in), "Failed to close %s.", fname);

    GLuint shader = glCreateShader(shaderType);//Compile the shader
    glShaderSource(shader, 1, (const GLchar**)&src, NULL);//...
    glCompileShader(shader);//...
    free(src); src = NULL;
    GLint test;//Check the result of the compilation
    glGetShaderiv(shader, GL_COMPILE_STATUS, &test);//...
    if(!test){//...
#define SHADER_COMP_LOG_SIZE 512
        char compilation_log[SHADER_COMP_LOG_SIZE];
        glGetShaderInfoLog(shader, SHADER_COMP_LOG_SIZE, NULL, compilation_log);
        log_err("Shader compilation of %s failed with this message: %s.", fname, compilation_log);
        goto error;
    }
    return shader;
error:
    free(src); src = NULL;
    if(in)fclose(in);
    error_exit();
    return 1;
}

GLuint create_program(const char *path_vert_shader, const char *path_frag_shader){
    GLuint vertexShader = load_and_compile_shader(path_vert_shader, GL_VERTEX_SHADER);
    GLuint fragmentShader = load_and_compile_shader(path_frag_shader, GL_FRAGMENT_SHADER);
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);
    return shaderProgram;
}
