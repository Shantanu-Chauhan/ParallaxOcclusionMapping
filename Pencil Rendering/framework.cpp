///////////////////////////////////////////////////////////////////////
// Provides the framework for graphics projects, mostly just GLUT
// calls to open a window and hook up various callbacks for
// mouse/keyboard interaction,  screen resizes, and redisplays.
////////////////////////////////////////////////////////////////////////

#include "framework.h"

Scene scene;

////////////////////////////////////////////////////////////////////////
// Do the OpenGL/GLut setup and then enter the interactive loop.
int main(int argc, char** argv)
{
	GLFWwindow* window;
	//glfwSetErrorCallback(error_callback);
	if (!glfwInit())
	{
		 //Initialization failed
	}
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

	window = glfwCreateWindow(750, 750, "Simple example", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	if (!window)	
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glewExperimental = true;
	glewInit();
	
	scene.width = 750;
	scene.height = 750;
    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("Rendered by: %s\n", glGetString(GL_RENDERER));
    fflush(stdout);

	while (true)
	{
		glfwPollEvents();
	}
    // Initialize interaction and the scene to be drawn.
    //InitInteraction();
    //scene.InitializeScene();

    // Enter the event loop.
    //glutMainLoop();
}

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}