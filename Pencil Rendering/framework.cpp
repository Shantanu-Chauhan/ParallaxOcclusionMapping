///////////////////////////////////////////////////////////////////////
// Provides the framework for graphics projects, mostly just GLUT
// calls to open a window and hook up various callbacks for
// mouse/keyboard interaction,  screen resizes, and redisplays.
////////////////////////////////////////////////////////////////////////

#include "framework.h"
#include<glew.h>
#include<imgui.h>
#include<imgui_impl_glfw.h>
#include<imgui_impl_opengl3.h>
Scene scene;

////////////////////////////////////////////////////////////////////////
// Do the OpenGL/GLut setup and then enter the interactive loop.
int main(int argc, char** argv)
{
	GLFWwindow* window;

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

	window = glfwCreateWindow(1024, 720, "Deffered Shading", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	if (!window)	
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glewExperimental = true;
	glewInit();
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	scene.width = 1024;
	scene.height = 720;
    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("Rendered by: %s\n", glGetString(GL_RENDERER));
    fflush(stdout);
	const char* glsl_version = "#version 330";
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

    InitInteraction(window,&io);
    scene.InitializeScene();
	int selection = 0;
	int rows=scene.numberOfRows, columns=scene.numberOfColumns, number = scene.numberoflights;
	float height = scene.lightHeight;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Deffered Shading");
		bool selected = true;
		ImGui::Checkbox("Local Lights", &scene.localLightsToggle);
		ImGui::Checkbox("Global Light ", &scene.globalLightToggle);
		std::string labels[5] = {"Lighting", "WorldPos","Diffuse","Specular","Normal" };
		const char* label = labels[selection].c_str();
		ImGui::InputInt("Number of Local Lights", &number,100);
		ImGui::InputInt("Number of Rows", &rows,5);
		ImGui::InputInt("Number of Columns", &columns,5);
		if(ImGui::Button("Apply", ImVec2(50.0f, 20.0f)))
		{
			scene.numberoflights = number;
			scene.numberOfColumns = columns;
			scene.numberOfRows = rows;
			scene.lightHeight = height;
			scene.CreateLights();
		}
		if (ImGui::BeginCombo("Select G - Buffer", label))
		{
			if (ImGui::Selectable("Lighting", selected))
			{
				scene.GBufferNum = 0;
				selection = 0;
			}
			if (ImGui::Selectable("WorldPos", selected))
			{
				scene.GBufferNum = 1;
				selection = 1;
			}
			if (ImGui::Selectable("Diffuse", selected))
			{
				scene.GBufferNum = 2;
				selection = 2;
			}
			if (ImGui::Selectable("Specular", selected))
			{
				scene.GBufferNum = 3;
				selection = 3;
			}
			if (ImGui::Selectable("Normal", selected))
			{
				scene.GBufferNum = 4;
				selection = 4;
			}
			ImGui::EndCombo();
		}
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
		ImGui::Render();
		scene.DrawScene();
		
		int display_w, display_h;
		/*glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.0f, 0.0f, 0.0, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);*/
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}
}