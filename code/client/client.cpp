#include <GL/glew.h>
// #include <GLFW/glfw3.h>


#include <iostream>

#include "imgui.h"
// #include "imgui_impl_glfw.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "imgui_impl_opengl3.h"

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"

#include "CarPhysics.h"

CarPhysics carPhysics;
CarPhysicsSerde carConfig(carPhysics);

// EXAMPLE CALLBACKS
// class MyCallbacks : public CallbackInterface {

// public:
// 	MyCallbacks(ShaderProgram& shader) : shader(shader) {}

// 	virtual void keyCallback(int key, int scancode, int action, int mods) {
// 		if (key == GLFW_KEY_R && action == GLFW_PRESS)
// 			shader.recompile();

// 		// press t to hot-reload car physics config
// 		if (key == GLFW_KEY_T && action == GLFW_PRESS)
// 			carConfig.deserialize();

// 		// press s to serialize current car config
// 		if (key == GLFW_KEY_S && action == GLFW_PRESS)
// 			carConfig.serialize();
// 	}

// 	virtual void cursorPosCallback(double xpos, double ypos) {
// 	}

// private:
// 	ShaderProgram& shader;
// };


std::vector<glm::vec3> drawFromMidpoints(std::vector<glm::vec3> src);
std::vector<glm::vec3> colourSquare(std::vector<glm::vec3> dest, glm::vec3 colour);

CPU_Geometry squaresDiamonds(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, int depth) {

	glm::vec3 squareColor(255 / 255, 255 / 255, 0 / 255);
	glm::vec3 diamondColor(255 / 255, 51 / 255, 153 / 255);

	CPU_Geometry cpu;

	cpu.verts.push_back(a);
	cpu.verts.push_back(b);
	cpu.verts.push_back(c);
	cpu.verts.push_back(d);
	
	cpu.cols = colourSquare(cpu.cols, squareColor);
	cpu.verts = drawFromMidpoints(cpu.verts);
	cpu.cols =colourSquare(cpu.cols, diamondColor);

	depth--;

	while (depth > 0)
	{

		for (int i = 0; i < 2; i++) {

			cpu.verts = drawFromMidpoints(cpu.verts);

			if (i == 0)
				cpu.cols = colourSquare(cpu.cols, squareColor);
			else cpu.cols = colourSquare(cpu.cols, diamondColor);


		}

		depth--;
	}

	return cpu;
}


std::vector<glm::vec3> drawFromMidpoints(std::vector<glm::vec3> src)
{

	std::vector<glm::vec3> output = src;

	int index = src.size() - 4;

	glm::vec3 a, b, c, d, e, f, g,h;

	a = output[index++];
	b = output[index++];
	c = output[index++];
	d = output[index];

	e = (0.5f * a + 0.5f * b);
	f = (0.5f * b + 0.5f * c);
	g = (0.5f * c + 0.5f * d);
	h = (0.5f * d + 0.5f * a);

	output.push_back(e);
	output.push_back(f);
	output.push_back(g);
	output.push_back(h);

	return output;
}

std::vector<glm::vec3> colourSquare(std::vector<glm::vec3> dest, glm::vec3 colour)
{
	std::vector<glm::vec3> colours = dest;

	for (int i = 0; i < 4; i++) {
		colours.push_back(colour);
	}

	return colours;


}

int main(int argc, char* argv[]) {
	Log::debug("Starting main");

	// WINDOW
	SDL_Init(SDL_INIT_EVERYTHING);
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired

	GLDebug::enable();

	// SHADERS
	// ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// CALLBACKS
	// window.setCallbacks(std::make_shared<MyCallbacks>(shader)); // can also update callbacks to new ones


	// GEOMETRY
	// CPU_Geometry cpuGeom;
	// GPU_Geometry gpuGeom;

	// cpuGeom = squaresDiamonds(glm::vec3(0.8, -0.8, 0), glm::vec3(0.8, 0.8, 0), glm::vec3(-0.8, 0.8, 0), glm::vec3(-0.8, -0.8, 0), 3);


	// for (int i = 0; i < cpuGeom.verts.size(); i++) {
	// 	std::cout << cpuGeom.verts[i] << std::endl;
	// }

	// gpuGeom.setVerts(cpuGeom.verts);
	// gpuGeom.setCols(cpuGeom.cols);

	// carConfig.deserialize();

	// NOTE(beau): put this somewhere else
	// It's not in the window constructor because input won't
	// work unless this code is called after the window callbacks are set
	// IMGUI_CHECKVERSION();
	// ImGui::CreateContext();
	// ImGuiIO& io = ImGui::GetIO(); (void)io;

	// ImGui::StyleColorsDark();

	// ImGui_ImplOpenGL3_Init("#version 330");
	// ImGui_ImplGlfw_InitForOpenGL(window.window.get(), true);

	// RENDER LOOP
	// while (!window.shouldClose()) {
	bool quit = false;
	SDL_Event event;
	while (!quit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = true;
			}
		}

		// glfwPollEvents();

		// shader.use();
		// gpuGeom.bind();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//std::cout << GLsizei(cpuGeom.verts.size());

		// for (int i = 0; i < GLsizei(cpuGeom.verts.size()); i+=4)
		// {
		// 	glDrawArrays(GL_LINE_LOOP, i, 4);
		// }

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		// ImGui_ImplOpenGL3_NewFrame();
		// ImGui_ImplGlfw_NewFrame();
		// ImGui::NewFrame();

		// ImGui::Begin("Car Physics", nullptr);

		// ImGui::SliderFloat("acceleration", &carPhysics.m_acceleration, 0.f, 1000.f);
		// ImGui::SliderFloat("suspension", &carPhysics.m_suspension_force, 0.f, 1000.f);

		// ImGui::End();

		// ImGui::Render();
		// ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// window.swapBuffers();
		SDL_GL_SwapWindow(window.window.get());
	}

	// ImGui_ImplOpenGL3_Shutdown();
	// ImGui_ImplGlfw_Shutdown();
	// ImGui::DestroyContext();

	SDL_Quit();
	// glfwTerminate();
	return 0;
}
