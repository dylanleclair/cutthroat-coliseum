#include <iostream>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "Window.h"

#include "systems/ecs.h"

#include "CarPhysics.h"

#include "GraphicsSystem.h"
#include "FrameCounter.h"

#include "Physx.h"

CarPhysics carPhysics;
CarPhysicsSerde carConfig(carPhysics);



struct ExampleComponent
{
	int a{ 0 };
	int b{ 0 };
	int c{ 0 };
};


struct ExampleSystem : ecs::ISystem
{

	virtual void Initialize() {}
	virtual void Teardown() {}
	virtual void Update(ecs::Scene &scene, float deltaTime)
	{
		for (Guid entityGuid : ecs::EntitiesInScene<ExampleComponent>(scene))
		{
			// pass in the guid of the entity to lookup the corresponding component
			ExampleComponent& ex = scene.GetComponent<ExampleComponent>(entityGuid);

			// do some operation of the data
			ex.a += 1;
			ex.b += 2;
			ex.c += 3;

			display = "ecs component: " + std::to_string(ex.a) + " " + std::to_string(ex.b) + " " + std::to_string(ex.c) + "!";
		}
	}

	std::string getDisplayString()
	{
		return display;
	}

private:
	std::string display{};
};

int main(int argc, char* argv[]) {
	Log::debug("Starting main");

	SDL_Init(SDL_INIT_EVERYTHING); // initialize all sdl systems
	Window window(800, 800, "Maximus Overdrive");

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	GLDebug::enable();

	// GEOMETRY
	CPU_Geometry cubeGeom;

	//make a wireframe cube
	cubeGeom.verts.push_back(glm::vec3(-1, -1, 1));
	cubeGeom.verts.push_back(glm::vec3(1, -1, 1));
	cubeGeom.verts.push_back(glm::vec3(1, 1, 1));
	cubeGeom.verts.push_back(glm::vec3(-1, -1, 1));
	cubeGeom.verts.push_back(glm::vec3(1, 1, 1));
	cubeGeom.verts.push_back(glm::vec3(-1, 1, 1));
	cubeGeom.verts.push_back(glm::vec3(1, -1, 1));
	cubeGeom.verts.push_back(glm::vec3(1, -1, -1));
	cubeGeom.verts.push_back(glm::vec3(1, 1, -1));
	cubeGeom.verts.push_back(glm::vec3(1, -1, 1));
	cubeGeom.verts.push_back(glm::vec3(1, 1, -1));
	cubeGeom.verts.push_back(glm::vec3(1, 1, 1));
	cubeGeom.verts.push_back(glm::vec3(1, -1, -1));
	cubeGeom.verts.push_back(glm::vec3(-1, -1, -1));
	cubeGeom.verts.push_back(glm::vec3(-1, 1, -1));
	cubeGeom.verts.push_back(glm::vec3(1, -1, -1));
	cubeGeom.verts.push_back(glm::vec3(-1, 1, -1));
	cubeGeom.verts.push_back(glm::vec3(1, 1, -1));
	cubeGeom.verts.push_back(glm::vec3(-1, -1, -1));
	cubeGeom.verts.push_back(glm::vec3(-1, -1, 1));
	cubeGeom.verts.push_back(glm::vec3(-1, 1, 1));
	cubeGeom.verts.push_back(glm::vec3(-1, -1, -1));
	cubeGeom.verts.push_back(glm::vec3(-1, 1, 1));
	cubeGeom.verts.push_back(glm::vec3(-1, 1, -1));
	cubeGeom.verts.push_back(glm::vec3(-1, 1, 1));
	cubeGeom.verts.push_back(glm::vec3(1, 1, 1));
	cubeGeom.verts.push_back(glm::vec3(1, 1, -1));
	cubeGeom.verts.push_back(glm::vec3(-1, 1, 1));
	cubeGeom.verts.push_back(glm::vec3(1, 1, -1));
	cubeGeom.verts.push_back(glm::vec3(-1, 1, -1));
	cubeGeom.verts.push_back(glm::vec3(1, -1, 1));
	cubeGeom.verts.push_back(glm::vec3(-1, -1, -1));
	cubeGeom.verts.push_back(glm::vec3(1, -1, -1));
	cubeGeom.verts.push_back(glm::vec3(1, -1, 1));
	cubeGeom.verts.push_back(glm::vec3(-1, -1, 1));
	cubeGeom.verts.push_back(glm::vec3(-1, -1, -1));

	for (int i = 0; i < 12; i++) {
		float col1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float col2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float col3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		for(int j = 0; j < 3; j++)
			cubeGeom.cols.push_back(glm::vec3(col1, col2, col3));
	}

	carConfig.deserialize();

	

	// init ecs 
	ecs::Scene mainScene;

	//FOR DYLAN: Why does having only 1 entity not seem to work? The component is added but it isn't being regiesterd,
	// In general it seems that you need n+1 entities to get n entities working.
	// spawn some entities.
	for (int i = 0; i < 2; i++)
	{
		ecs::Entity e = mainScene.CreateEntity();
		mainScene.AddComponent(e.guid, ExampleComponent{ 0,1,2 });



		
		//yes, I know this makes a memory leak. It is just to test
		CPU_Geometry* cpuGeom = new CPU_Geometry();
		//put the meshes of 500 cubes in one entity
		for (int j = 0; j < 500; j++) {
			//create cube meshes for them
			glm::vec3 temp;
			const float LO = -3;
			const float HI = 3;
			for (int j = 0; j < 3; j++)
				temp[j] = LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
			Position* position = new Position(temp);
			//do some sample calculations to simulate static meshes
			for (int j = 0; j < cubeGeom.verts.size(); j++) {
				cpuGeom->verts.push_back(position->getTransformMatrix() * glm::vec4(cubeGeom.verts[j], 1));
				cpuGeom->cols.push_back(cubeGeom.cols[j]);
			}
		}
		//create cube meshes for them
		glm::vec3 temp;
		const float LO = -1;
		const float HI = 1;
		for (int j = 0; j < 3; j++)
			temp[j] = LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
		Position* position = new Position(temp);
		mainScene.AddComponent(e.guid, RenderComponent{ cpuGeom, position });
	}
	
	// create instance of system to use.
	ExampleSystem exampleEcsSystem;
	GraphicsSystem gs(window);

	init_physx();

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
		

		// BEGIN ECS SYSTEMS UPDATES 
		exampleEcsSystem.Update(mainScene, 0.0f);
		gs.Update(mainScene, 0.0f);
		// END__ ECS SYSTEMS UPDATES

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		// BEGIN FRAMERATE COUNTER
		ImGui::SetNextWindowSize(ImVec2(500, 100)); 
		ImGui::Begin("Milestone 1");
		ImGui::Text("framerate: %d", framerate.framerate());
        ImGui::PlotLines("Frametime plot (ms)", framerate.m_time_queue.data(), framerate.m_time_queue.size());
        ImGui::PlotLines("Framerate plot (hz)", framerate.m_rate_queue.data(), framerate.m_rate_queue.size());
		// TODO(milestone 1): display physx value as proof that physx is initialized
		ImGui::End();
		// END FRAMERATE COUNTER

		// simulate physics with time delta = time of last frame
		// XXX(beau): DOES NOT CLAMP TIME DELTA
		// TODO(beau): make a setup for dealing with time - follow slides
		{
			float frame_time_seconds = framerate.m_time_queue.front() / 1000.0f;
			gScene->simulate(frame_time_seconds);
			gScene->fetchResults(true);
		}

		// TODO(milestone 1): strip all non-milestone related imgui windows out
		// BEGIN CAR PHYSICS PANEL
		ImGui::Begin("Car Physics", nullptr);
		ImGui::SliderFloat("acceleration", &carPhysics.m_acceleration, 0.f, 1000.f);
		ImGui::SliderFloat("suspension", &carPhysics.m_suspension_force, 0.f, 1000.f);
		if (ImGui::Button("Serialize")) carConfig.serialize();
		ImGui::End();
		// END CAR PHYSICS PANEL

		// BEGIN ECS DEMO PANEL
		ImGui::Begin("ECS Demo");
		ImGui::Text("%s", exampleEcsSystem.getDisplayString().c_str());
		ImGui::End();

		// NOTE: the imgui bible - beau
		ImGui::ShowDemoWindow();

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
