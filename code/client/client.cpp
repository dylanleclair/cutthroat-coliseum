#include <iostream>


#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "graphics/Geometry.h"

#include "Window.h"

#include "systems/ecs.h"
#include "systems/GraphicsSystem.h"
#include "systems/components.h"

#include "FrameCounter.h"

glm::vec3 calculateSpherePoint(float s, float t)
{
	float z = cos(2 * M_PI * t) * sin(M_PI * s);
	float x = sin(2 * M_PI * t) * sin(M_PI * s);
	float y = cos(M_PI * s);
	return(glm::vec3(x, y, z));
}

void createSphere(CPU_Geometry* geom, int resolution)
{
	const float sInc = 1.0f / resolution;
	const float tInc = 1.0f / resolution;
	for (float s = 0; s < 1 - 0.001; s += sInc)
	{
		for (float t = 0; t < 1; t += tInc)
		{
			geom->verts.push_back(calculateSpherePoint(t, s));
			geom->verts.push_back(calculateSpherePoint(t + tInc, s));
			geom->verts.push_back(calculateSpherePoint(t + tInc, s + sInc));
			geom->verts.push_back(calculateSpherePoint(t, s));
			geom->verts.push_back(calculateSpherePoint(t, s + sInc));
			geom->verts.push_back(calculateSpherePoint(t + tInc, s + sInc));

			glm::vec2 C1 = glm::vec2(1 - s, 1 - t);
			glm::vec2 C3 = glm::vec2(1 - (s + sInc), (1 - t));
			glm::vec2 C2 = glm::vec2(1 - s, 1 - (t + tInc));
			glm::vec2 C4 = glm::vec2(1 - (s + sInc), 1 - (t + tInc));

			geom->texs.push_back(C1);
			geom->texs.push_back(C2);
			geom->texs.push_back(C4);

			geom->texs.push_back(C1);
			geom->texs.push_back(C3);
			geom->texs.push_back(C4);


			geom->norms.push_back(calculateSpherePoint(t, s));
			geom->norms.push_back(calculateSpherePoint(t + tInc, s));
			geom->norms.push_back(calculateSpherePoint(t + tInc, s + sInc));
			geom->norms.push_back(calculateSpherePoint(t, s));
			geom->norms.push_back(calculateSpherePoint(t, s + sInc));
			geom->norms.push_back(calculateSpherePoint(t + tInc, s + sInc));

		}
	}
}

int main(int argc, char* argv[]) {
	printf("Starting main");


	SDL_Init(SDL_INIT_EVERYTHING); // initialize all sdl systems
	Window window(1200, 800, "Maximus Overdrive");

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// create instance of system to use.
	GraphicsSystem gs(window);
	//init_physics();

	// init ecs 
	ecs::Scene mainScene;

	
	//make an entity
	ecs::Entity torus = mainScene.CreateEntity();
	CPU_Geometry torus_geom;
	RenderComponent torus_rend = RenderComponent();
	GraphicsSystem::importOBJ(torus_rend, "cube.obj");
	TransformComponent torus_trans = TransformComponent();
	mainScene.AddComponent(torus.guid, torus_rend);
	mainScene.AddComponent(torus.guid, torus_trans);


	FramerateCounter framerate;


	bool quit = false;
	int controlledCamera = 0;
  
  
	// GAME LOOP
	while (!quit) {
		//polls all pending input events until there are none left in the queue
		while (SDL_PollEvent(&window.event)) {
			ImGui_ImplSDL2_ProcessEvent(&window.event);

			if (window.event.type == SDL_QUIT)
				quit = true;

			if (window.event.type == SDL_KEYDOWN) {
				switch (window.event.key.keysym.sym) {
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

					// Prinout of camera matrix
					case SDLK_c:
						std::cout << gs.getCameraView()[0][0] << ", " << gs.getCameraView()[0][1] << ", " << gs.getCameraView()[0][2] << ", " << gs.getCameraView()[0][3] << "," << std::endl;
						std::cout << gs.getCameraView()[1][0] << ", " << gs.getCameraView()[1][1] << ", " << gs.getCameraView()[1][2] << ", " << gs.getCameraView()[1][3] << "," << std::endl;
						std::cout << gs.getCameraView()[2][0] << ", " << gs.getCameraView()[2][1] << ", " << gs.getCameraView()[2][2] << ", " << gs.getCameraView()[2][3] << "," << std::endl;
						std::cout << gs.getCameraView()[3][0] << ", " << gs.getCameraView()[3][1] << ", " << gs.getCameraView()[3][2] << ", " << gs.getCameraView()[3][3] << std::endl;
						std::cout << std::endl;
						break;
					case SDLK_ESCAPE:	// (Pressing escape closes the window, useful for fullscreen);
						quit = true;
						break;
					default:
						break;
				};
			}

			//pass the event to the camera
			gs.input(window.event, controlledCamera);
		}
		
		gs.Update(mainScene, 0.0f);

		// END__ ECS SYSTEMS UPDATES

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		// BEGIN FRAMERATE COUNTER
		ImGui::SetNextWindowSize(ImVec2(500, 100)); 
		ImGui::Begin("Milestone 2");
		ImGui::Text("framerate: %d", framerate.framerate());
        ImGui::PlotLines("Frametime plot (ms)", framerate.m_time_queue.data(), framerate.m_time_queue.size());
        ImGui::PlotLines("Framerate plot (hz)", framerate.m_rate_queue.data(), framerate.m_rate_queue.size());
		// TODO(milestone 1): display physx value as proof that physx is initialized
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
