/*
 *  C11+
 */


#include "chores.h"
#include "render.h"
#include <assert.h>
#include <stdlib.h>

static_assert(GLFW_VERSION_MAJOR == 2 && GLFW_VERSION_MINOR == 7,"Not using GLFW 2.7.X: later versions are incompatible and earlier versions MIGHT be.");

//void GLFWCALL window_resized(int width, int height){//Called when the window is resized
//	glViewport(0, 0, width, height);
//	display();
//}

static int pause = 0, step_frame = 0;

void keyboard(int key, int action){//Called for keyboard events
    if(GLFW_PRESS == action)switch(key){
        case 'F':
            step_frame = step_frame ? 0 : 1;
            break;
        case 'P':
            pause = pause ? 0 : 1;
            break;
        case 'Q':
            glfwTerminate();
            exit(0);
            break;//Not really necessary, but...
        default:
            renderer_specific_commands(key);
            break;
    }
}

void GLFWCALL expose(void){
    glfwSwapBuffers();//How to fix jiggle? TODO
}

int main(const int argc, const char *const *const argv){
    if(argc < 2){
        fputs("Need path of file to render.\n\0",stderr);
        return 1;
    }
    if (!glfwInit()){
        fputs("GLFW init failed.\n\0",stderr);
        exit(EXIT_FAILURE);
    }

//    // Use OpenGL 3.3 core profile
//    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//    glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
//    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);

    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);

    unsigned int win_x = 1024, win_y = 1024;

    if(!glfwOpenWindow(win_x, win_y, 8, 8, 8, 0, 8, 8, GLFW_WINDOW)){//Open a window and attach an OpenGL rendering context to the window surface
        fputs("Window opening failed.\n\0",stderr);
        error_exit();
    }

    glfwSetWindowTitle(argv[0]);

    int major, minor, rev;//Print the OpenGL version
    glfwGetGLVersion(&major, &minor, &rev);//...
    printf("OpenGL - %d.%d.%d\n",major,minor,rev);//...
    assert(major > 2);

    glewExperimental = GL_TRUE;//Initialize GLEW
    if(glewInit() != GLEW_OK){//...
        fputs("Failed to initialize GLEW!\n\0",stderr);
        error_exit();
    }

//    printf("%s %d\n",__FILE__,__LINE__);
    initialize(win_x, win_y, argv, argc);
    //glfwSetWindowSizeCallback(window_resized);//Register a callback function for window resize events
    glfwSetKeyCallback(keyboard);//Register a callback function for keyboard pressed events
    glfwSetWindowRefreshCallback(expose);
    //glfwSwapInterval(1);
    for(int running = GL_TRUE;running;running = glfwGetWindowParam(GLFW_OPENED)){
        if(pause)glfwWaitEvents();
        else if(display())break;
        if(step_frame)glfwWaitEvents();
        else glfwPollEvents();
    }
    glfwTerminate();//Terminate GLFW
    return 0;
}
