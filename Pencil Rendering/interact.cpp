////////////////////////////////////////////////////////////////////////
// All keyboard, mouse, and other interactions are implemented here.
// The single entry point, InitInteraction, sets up GLUT callbacks for
// various events that an interactive graphics program needs to
// handle.
//
// The various callbacks are:
//    glutDisplayFunc: called repeatedly to redraw the scene every 30th of a second.
//    glutReshapeFunc: called whenever the window changes size
//    glutKeyboardFunc and glutKeyboardUpFunc: called whenever a key is pressed or released
//    glutMouseFunc:  called when a mouse button is pressed or released
//    glutMotionFunc: called when the mouse is moved

#include "framework.h"
#include <imgui.h>
const float PI = 3.14159f;
extern Scene scene;       // Declared in framework.cpp, but used here.

// Some globals used for mouse handling.
int mouseX, mouseY;
bool shifted = false;
bool leftDown = false;
bool middleDown = false;
bool rightDown = false;

ImGuiIO* io;
////////////////////////////////////////////////////////////////////////
// Function called to exit
void Quit(void* clientData)
{

}

////////////////////////////////////////////////////////////////////////
// Called by GLUT when the window size is changed.
void ReshapeWindow(int w, int h)
{
	if (w && h) {
		glViewport(0, 0, w, h);
	}
	scene.width = w;
	scene.height = h;
	printf("%d width , %d height\n", scene.width, scene.height);
}

////////////////////////////////////////////////////////////////////////
// Called by GLut for keyboard actions.
//void KeyboardDown(unsigned char key, int x, int y)
void KeyboardDown(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (io->WantCaptureKeyboard == true)
		return;
	int state = glfwGetKey(window, key);

	fflush(stdout);
	
	if (state == GLFW_PRESS)
	{
		//printf("key down %c(%d)\n", key, key);
		switch (key) {
		case GLFW_KEY_W:
			scene.w = true;
			break;
		case GLFW_KEY_S:
			scene.s = true;

			break;
		case GLFW_KEY_D:
			scene.d = true;

			break;
		case GLFW_KEY_A:
			scene.a = true;
			break;
		case GLFW_KEY_E:
			scene.eyee = !scene.eyee;//trigger the eye controls
			break;
		case GLFW_KEY_LEFT_SHIFT:
			shifted = true;
			break;
			case GLFW_KEY_Q: case GLFW_KEY_ESCAPE:       // Escape and 'q' keys quit the application
			exit(0);
		}
	}
	else
		if (state == GLFW_RELEASE)
		{
			//printf("key up %c(%d)\n", key, key);
			switch (key)
			{
			case GLFW_KEY_W:
				scene.w = false;

				break;
			case GLFW_KEY_S:
				scene.s = false;

				break;
			case GLFW_KEY_D:
				scene.d = false;

				break;
			case GLFW_KEY_A:
				scene.a = false;
				break;
			case GLFW_KEY_LEFT_SHIFT:
				shifted = false;
				break;
			}
		}
}


////////////////////////////////////////////////////////////////////////
// Called by GLut when a mouse button changes state.
//void MouseButton(int button, int state, int x, int y)
void MouseButton(GLFWwindow* window, int button, int action, int mods)
{
	
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		leftDown = (action == GLFW_PRESS);
		//printf("Left button down\n");
	}

	else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
		middleDown = (action == GLFW_PRESS);
		//printf("Middle button down\n");
	}

	else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		rightDown = (action == GLFW_PRESS);
		//printf("Right button down\n");
	}

	if (io->WantCaptureKeyboard == true)
		return;

	fflush(stdout);
}

////////////////////////////////////////////////////////////////////////
// Called by GLut when a mouse moves (while a button is down)
//void MouseMotion(int x, int y)
void MouseMotion(GLFWwindow* window, double x, double y)
{
	if (io->WantCaptureKeyboard == true)
		return;
	// Calculate the change in the mouse position
	int dx = x - mouseX;
	int dy = y - mouseY;

	if (leftDown && shifted) {  // Rotate light position
		scene.lightSpin += dx / 3.0f;
		scene.lightTilt -= dy / 3.0f;
	}
	if (leftDown && !shifted) {
		scene.spin += dx * 0.30f;
		scene.tilt += dy * 0.30f;
	}
	if (rightDown && !scene.eyee)
	{
		scene.tx -= dx * 0.10f;
		scene.ty += dy * 0.10f;
	}

	if (middleDown && shifted) {
		scene.lightDist = pow(scene.lightDist, 1.0f - dy / 200.0f);
	}

	else if (middleDown) {}

	if (rightDown) {
	}

	// Record this position
	mouseX = x;
	mouseY = y;

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (io->WantCaptureKeyboard == true)
		return;
	// Test if the SHIFT key was down for this mouse click

	// Figure out the mouse action, and handle accordingly
	if (yoffset>0 && shifted) { // Scroll light in
		scene.lightDist = pow(scene.lightDist, 1.0f / 1.02f);
		//printf("shifted scroll up\n");
	}

	else if (yoffset<0 && shifted) { // Scroll light out
		scene.lightDist = pow(scene.lightDist, 1.02f);
		//printf("shifted scroll down\n");
	}

	else if (yoffset>0) {
		scene.zoom /= 1.2f;

		//printf("scroll up\n");
	}

	else if (yoffset<0 ) {
		scene.zoom *= 1.2f;
		//printf("scroll down\n");
	}
}


void InitInteraction(GLFWwindow* window, ImGuiIO* IO)
{
	io = IO;
	glfwSetKeyCallback(window, KeyboardDown);
	//glutIgnoreKeyRepeat(true);  // DO NOT change or disable this line!
	glfwSetCursorPosCallback(window, MouseMotion);
	glfwSetMouseButtonCallback(window, MouseButton);
	glfwSetScrollCallback(window, scroll_callback);

}
