#include <GL/glew.h>


#include <iostream>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"

#include "CarPhysics.h"

#include "GraphicsSystem.h"

CarPhysics carPhysics;
CarPhysicsSerde carConfig(carPhysics);


int main(int argc, char* argv[]) {
	Log::debug("Starting main");

	SDL_Init(SDL_INIT_EVERYTHING); // initialize all sdl systems
	Window window(800, 800, "CPSC 453");

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	GLDebug::enable();

	// GEOMETRY
	CPU_Geometry cpuGeom;

	//make a wireframe cube
	cpuGeom.verts.push_back(glm::vec3(-1, -1, 1));
	cpuGeom.verts.push_back(glm::vec3(1, -1, 1));
	cpuGeom.verts.push_back(glm::vec3(1, 1, 1));
	cpuGeom.verts.push_back(glm::vec3(-1, -1, 1));
	cpuGeom.verts.push_back(glm::vec3(1, 1, 1));
	cpuGeom.verts.push_back(glm::vec3(-1, 1, 1));
	cpuGeom.verts.push_back(glm::vec3(1, -1, 1));
	cpuGeom.verts.push_back(glm::vec3(1, -1, -1));
	cpuGeom.verts.push_back(glm::vec3(1, 1, -1));
	cpuGeom.verts.push_back(glm::vec3(1, -1, 1));
	cpuGeom.verts.push_back(glm::vec3(1, 1, -1));
	cpuGeom.verts.push_back(glm::vec3(1, 1, 1));
	cpuGeom.verts.push_back(glm::vec3(1, -1, -1));
	cpuGeom.verts.push_back(glm::vec3(-1, -1, -1));
	cpuGeom.verts.push_back(glm::vec3(-1, 1, -1));
	cpuGeom.verts.push_back(glm::vec3(1, -1, -1));
	cpuGeom.verts.push_back(glm::vec3(-1, 1, -1));
	cpuGeom.verts.push_back(glm::vec3(1, 1, -1));
	cpuGeom.verts.push_back(glm::vec3(-1, -1, -1));
	cpuGeom.verts.push_back(glm::vec3(-1, -1, 1));
	cpuGeom.verts.push_back(glm::vec3(-1, 1, 1));
	cpuGeom.verts.push_back(glm::vec3(-1, -1, -1));
	cpuGeom.verts.push_back(glm::vec3(-1, 1, 1));
	cpuGeom.verts.push_back(glm::vec3(-1, 1, -1));
	cpuGeom.verts.push_back(glm::vec3(-1, 1, 1));
	cpuGeom.verts.push_back(glm::vec3(1, 1, 1));
	cpuGeom.verts.push_back(glm::vec3(1, 1, -1));
	cpuGeom.verts.push_back(glm::vec3(-1, 1, 1));
	cpuGeom.verts.push_back(glm::vec3(1, 1, -1));
	cpuGeom.verts.push_back(glm::vec3(-1, 1, -1));
	cpuGeom.verts.push_back(glm::vec3(1, -1, 1));
	cpuGeom.verts.push_back(glm::vec3(-1, -1, -1));
	cpuGeom.verts.push_back(glm::vec3(1, -1, -1));
	cpuGeom.verts.push_back(glm::vec3(1, -1, 1));
	cpuGeom.verts.push_back(glm::vec3(-1, -1, 1));
	cpuGeom.verts.push_back(glm::vec3(-1, -1, -1));

	for (int i = 0; i < 12; i++) {
		float col1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float col2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float col3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		for(int j = 0; j < 3; j++)
			cpuGeom.cols.push_back(glm::vec3(col1, col2, col3));
	}

	carConfig.deserialize();

	GraphicsSystem gs(window);

	render_packet pack(cpuGeom, Position());
	gs.addPrimitive(pack);

	// GAME LOOP
	bool quit = false;
	int controlledCamera = 0;
	while (!quit) {
		//polls all pending input events until there are none left in the queue
		while (SDL_PollEvent(&window.event)) {
			ImGui_ImplSDL2_ProcessEvent(&window.event);

			if (window.event.type == SDL_QUIT)
				quit = true;

			if (window.event.type == SDL_KEYDOWN) {
				switch (window.event.key.keysym.sym) {
					case SDLK_r:
						//TODO recompile the shader
						break;
					case SDLK_t:
						carConfig.deserialize();
						break;
					case SDLK_s:
						carConfig.serialize();
						break;
					case SDLK_0:
						controlledCamera = 0;
						break;
					case SDLK_1:
						controlledCamera = 1;
						break;
					case SDLK_2:
						controlledCamera = 2;
						break;
					case SDLK_3:
						controlledCamera = 3;
						break;
					default:
						break;
				};
			}

			//pass the event to the camera
			gs.input(window.event, controlledCamera);
		}

		gs.render();

		//glEnable(GL_FRAMEBUFFER_SRGB);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Car Physics", nullptr);

		ImGui::SliderFloat("acceleration", &carPhysics.m_acceleration, 0.f, 1000.f);
		ImGui::SliderFloat("suspension", &carPhysics.m_suspension_force, 0.f, 1000.f);

		ImGui::End();

		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.swapBuffers();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_Quit();
	return 0;
}
