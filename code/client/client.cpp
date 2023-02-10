#include <iostream>


#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"


#include "graphics/Geometry.h"
#include "PxPhysicsAPI.h"

#include "Window.h"

#include "systems/ecs.h"
#include "systems/GraphicsSystem.h"
#include "systems/components.h"

#include "CarPhysics.h"
#include "FrameCounter.h"


using namespace physx;

extern PxRigidBody* getVehicleRigidBody();
extern bool initPhysics();
extern void stepPhysics(SDL_GameController* controller, float timestep = 1 / 164.f);
extern void cleanupPhysics();
extern int carSampleInit();

extern PxScene* gScene;

CarPhysics carPhysics;
CarPhysicsSerde carConfig(carPhysics);




int main(int argc, char* argv[]) {
	printf("Starting main");


	carSampleInit();

	SDL_Init(SDL_INIT_EVERYTHING); // initialize all sdl systems
	Window window(800, 800, "Maximus Overdrive");

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	carConfig.deserialize();

	// create instance of system to use.
	GraphicsSystem gs(window);
	//init_physics();

	// init ecs 
	ecs::Scene mainScene;
	
	
	std::cout << "Component initalization finished\n";

	
	if (initPhysics())
	{
		std::cout << "initialized physx driving model\n";
	}


	//ground
	ecs::Entity ground_e = mainScene.CreateEntity();
	CPU_Geometry ground_geom;
	glm::vec3 square[] = {
		{1.f, 0.f, 1.0f},
		{1.f, 0.f, -1.0f},
		{-1.f, 0.f, -1.0f},

		{1.f, 0.f, 1.0f},
		{-1.f, 0.f, -1.0f},
		{-1.f, 0.f, 1.0f},
	};

	const float scale = 5.f;
	for (int x = -30; x <= 30; x++) {
		for (int z = -30; z <= 30; z++) {
			for (int i = 0; i < 6; i++) {
				ground_geom.verts.push_back(square[i]*scale + glm::vec3(x * scale,-1,z * scale));
				ground_geom.cols.push_back(glm::vec3(0, 1, 0));
			}
		}
	}

	RenderComponent ground = RenderComponent(&ground_geom);
	ground.appearance = 1;
	mainScene.AddComponent(ground_e.guid, ground);

	TransformComponent trans2 = TransformComponent();
	mainScene.AddComponent(ground_e.guid, trans2);
	
	//make an entity
	ecs::Entity e = mainScene.CreateEntity();
	ecs::Entity level_e = mainScene.CreateEntity();
	ecs::Entity outWall_e = mainScene.CreateEntity();
	ecs::Entity inWall_e = mainScene.CreateEntity();

	// Car
	RenderComponent rend = RenderComponent();
	GraphicsSystem::readVertsFromFile(rend, "models/test_car.obj");
	mainScene.AddComponent(e.guid, rend);
	TransformComponent trans = TransformComponent(getVehicleRigidBody());
	mainScene.AddComponent(e.guid, trans);


	//finish box
	ecs::Entity finish_e = mainScene.CreateEntity();
	CPU_Geometry finish_geom;

	glm::vec3 rectangle[] = {
		{10.f, 1.f, 0.0f},
		{10.f, -1.f, 0.0f},
		{-10.f, -1.f, 0.0f},

		{10.f, 1.f, 0.0f},
		{-10.f, -1.f, 0.0f},
		{-10.f, 1.f, 0.0f},
	};
	for (int i = 0; i < 6; i++) {
		finish_geom.verts.push_back(rectangle[i]);
		finish_geom.cols.push_back(glm::vec3(1, 0, 0));
	}


	RenderComponent finish = RenderComponent(&finish_geom);
	finish.appearance = 0;
	mainScene.AddComponent(finish_e.guid, finish);

	TransformComponent trans3 = TransformComponent();
	trans3.setPosition(glm::vec3(10, 0, 0));
	mainScene.AddComponent(finish_e.guid, trans3);



	// Level
	RenderComponent level_r = RenderComponent();
	GraphicsSystem::readVertsFromFile(level_r, "models/large_test_torus.obj");
	mainScene.AddComponent(level_e.guid, level_r);	
	mainScene.AddComponent(level_e.guid, trans2);

	RenderComponent outWall = RenderComponent();
	GraphicsSystem::readVertsFromFile(outWall, "models/large_test_torus_inwall.obj");
	mainScene.AddComponent(outWall_e.guid, outWall);

	RenderComponent inWall = RenderComponent();
	GraphicsSystem::readVertsFromFile(inWall, "models/large_test_torus_outwall.obj");
	mainScene.AddComponent(inWall_e.guid, inWall);


	FramerateCounter framerate;

	assert(SDL_NumJoysticks() > 0);
	// TODO: handle no controller
	SDL_GameController* controller = nullptr;
	controller = SDL_GameControllerOpen(0);
	assert(controller);
	SDL_Joystick* joy = nullptr;
	joy = SDL_GameControllerGetJoystick(controller);
	assert(joy);
	int instanceID =  SDL_JoystickInstanceID(joy);


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

					// Prinout of camera matrix
					case SDLK_c:
						std::cout << gs.getCameraView()[0][0] << ", " << gs.getCameraView()[0][1] << ", " << gs.getCameraView()[0][2] << ", " << gs.getCameraView()[0][3] << std::endl;
						std::cout << gs.getCameraView()[1][0] << ", " << gs.getCameraView()[1][1] << ", " << gs.getCameraView()[1][2] << ", " << gs.getCameraView()[1][3] << std::endl;
						std::cout << gs.getCameraView()[2][0] << ", " << gs.getCameraView()[2][1] << ", " << gs.getCameraView()[2][2] << ", " << gs.getCameraView()[2][3] << std::endl;
						std::cout << gs.getCameraView()[3][0] << ", " << gs.getCameraView()[3][1] << ", " << gs.getCameraView()[3][2] << ", " << gs.getCameraView()[3][3] << std::endl;
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


		
		/*
		// BEGIN ECS SYSTEMS UPDATES
		//std::cout << "Beginning system updates\n";
		if(framerate.m_time_queue.size() != 0)
			gScene->simulate(framerate.m_time_queue.front() / 1000.0f);
		else
			gScene->simulate(0.1);
		gScene->fetchResults(true); //block until the simulation is finished
		*/
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
		// END FRAMERATE COUNTER

		// simulate physics with time delta = time of last frame
		// XXX(beau): DOES NOT CLAMP TIME DELTA
		// TODO(beau): make a setup for dealing with time - follow slides
		{
			//float frame_time_seconds = framerate.m_time_queue.front() / 1000.0f;
			//gScene->simulate(frame_time_seconds);
			//gScene->fetchResults(true);
		}

		// PHYSX DRIVER UPDATE
		stepPhysics(controller);


		// TODO(milestone 1): strip all non-milestone related imgui windows out
		// BEGIN CAR PHYSICS PANEL
		ImGui::Begin("Car Physics", nullptr);
		ImGui::SliderFloat("acceleration", &carPhysics.m_acceleration, 0.f, 1000.f);
		ImGui::SliderFloat("suspension", &carPhysics.m_suspension_force, 0.f, 1000.f);
		if (ImGui::Button("Serialize")) carConfig.serialize();
		ImGui::End();
		// END CAR PHYSICS PANEL
		
		// BEGIN A BUTTON THING
		bool cbutton = false;
		cbutton = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);
		ImGui::Begin("Buttons", nullptr);
		ImGui::Checkbox("a button", &cbutton);
		ImGui::End();
		// END A BUTTON THING

		// BEGIN JOYSTICK THING
		float axis = 0;
		axis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
		ImGui::Begin("Axes aka Joysticks and triggers");
		ImGui::Text("Right trigger: %hd", axis);
		ImGui::End();
		// END JOYSTICK THING



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
	

	cleanupPhysics();

	SDL_JoystickClose(joy);
	joy = nullptr;
	SDL_GameControllerClose(controller);
	controller = nullptr;

	SDL_Quit();
	return 0;
}
