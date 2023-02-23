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
#include "systems/ai.h"

#include "ImGuiDebug.h"

#include "utils/Time.h"
#include "Input.h"

#include "systems/PhysicsSystem.h"

#include "Car.h"

glm::vec3 calculateSpherePoint(float s, float t)
{
	float z = cos(2 * M_PI * t) * sin(M_PI * s);
	float x = sin(2 * M_PI * t) * sin(M_PI * s);
	float y = cos(M_PI * s);
	return(glm::vec3(x, y, z));
}

CarPhysics carPhysics;
CarPhysicsSerde carConfig(carPhysics);

int lapCount = 0;
bool isFinished = false;

uint32_t lastTime_millisecs;

void finishLinePrint() {
	lapCount++;
	std::cout << "Lap: " << lapCount << std::endl;
	if (lapCount == 2) {
		std::cout << "You win !" << std::endl;
	}
}


int main(int argc, char* argv[]) {
	printf("Starting main");




	SDL_Init(SDL_INIT_EVERYTHING); // initialize all sdl systems
	Window window(1200, 800, "Maximus Overdrive");

	lastTime_millisecs = SDL_GetTicks();

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	carConfig.deserialize();


	/**
	 * Begin initialization of ECS systems, entities, etc.
	 * - Graphics
	 * - Physics
	 * - AI
	 */

	// first and foremost, create a scene.
	ecs::Scene mainScene;

	GraphicsSystem gs(window);

	physics::PhysicsSystem physicsSystem{};
	physicsSystem.Initialize();






	// init ecs 


	std::cout << "Component initalization finished\n";


	//make an entity
	ecs::Entity car_e = mainScene.CreateEntity();
	ecs::Entity level_e = mainScene.CreateEntity();
	ecs::Entity outWall_e = mainScene.CreateEntity();
	ecs::Entity inWall_e = mainScene.CreateEntity();
	ecs::Entity ground_e = mainScene.CreateEntity();

	mainScene.AddComponent(car_e.guid, Car{});
	Car& testCar = mainScene.GetComponent<Car>(car_e.guid);
	testCar.physicsSystem = &physicsSystem;
	if (!testCar.initVehicle())
	{
		std::cout << "ERROR: could not initialize vehicle";
	}



	// Car
	RenderModel car_r = RenderModel();
	GraphicsSystem::importOBJ(car_r, "test_car.obj");
	car_r.setModelColor(glm::vec3(0.5f, 0.5f, 0.f));
	mainScene.AddComponent(car_e.guid, car_r);
	TransformComponent car_t = TransformComponent(testCar.getVehicleRigidBody());
	car_t.setPosition(glm::vec3(0, 1, 0));
	car_t.setRotation(glm::quat(0, 0, 0, 1));
	mainScene.AddComponent(car_e.guid, car_t);
	

	auto& car_render = mainScene.GetComponent<RenderModel>(car_e.guid);
	std::cout << "Car Guid: " << car_e.guid << std::endl;
	
	//finish box
	ecs::Entity finish_e = mainScene.CreateEntity();
	CPU_Geometry finish_geom;

	glm::vec3 rectangle[] = {
		{-10.f, 0.5f, 0.0f},
		{-10.f, -1.f, 0.0f},
		{-5.f, -1.f, 0.0f},

		{-5.f, 0.5f, 0.0f},
		{-5.f, -1.f, 0.0f},
		{-10.f, 0.5f, 0.0f},
	};
	for (int i = 0; i < 6; i++) {
		finish_geom.verts.push_back(rectangle[i]);
	}
	for (int i = 0; i < 6; i++) {
		finish_geom.verts.push_back(rectangle[5 - i]);
	}

	
	// Finish line components
	RenderModel finish = RenderModel();
	finish.attachMesh(finish_geom);
	mainScene.AddComponent(finish_e.guid, finish);

	TransformComponent finish_t = TransformComponent();
	finish_t.setPosition(glm::vec3(9, 1, -2));
	mainScene.AddComponent(finish_e.guid, finish_t);

	// Pathfinding
	PathfindingComponent car_pathfinder{ finish_e.guid };
	mainScene.AddComponent(car_e.guid, car_pathfinder);
	

	// Path renderer
	ecs::Entity path = mainScene.CreateEntity();
	mainScene.AddComponent(path.guid, TransformComponent{});
	mainScene.AddComponent(path.guid, RenderLine{});

	AISystem aiSystem{ path.guid };

	
	// Level
	TransformComponent level_t = TransformComponent();
	mainScene.AddComponent(ground_e.guid, level_t);

	RenderModel level_r = RenderModel();
	GraphicsSystem::importOBJ(level_r, "large_test_torus.obj");
	level_r.setModelColor(glm::vec3(0, 0, 1));
	mainScene.AddComponent(level_e.guid, level_r);
	mainScene.AddComponent(level_e.guid, level_t);

	TransformComponent wall_t = TransformComponent();
	wall_t.setPosition(glm::vec3(0, 2, 0));

	RenderModel outWall = RenderModel();
	GraphicsSystem::importOBJ(outWall, "large_test_torus_inwall.obj");
	outWall.setModelColor(glm::vec3(0.2f, 0.2f, 0.6f));
	mainScene.AddComponent(outWall_e.guid, outWall);
	mainScene.AddComponent(outWall_e.guid, wall_t);

	RenderModel inWall = RenderModel();
	GraphicsSystem::importOBJ(inWall, "large_test_torus_outwall.obj");
	inWall.setModelColor(glm::vec3(0.2f, 0.2f, 0.6f));
	mainScene.AddComponent(inWall_e.guid, inWall);
	mainScene.AddComponent(inWall_e.guid, wall_t);

	// This is how to change the position of the object after it has been passed to the ECS
	/*
	auto &wallTrans = mainScene.GetComponent<TransformComponent>(outWall_e.guid);
	wallTrans.setPosition(glm::vec3(0, 0, 0));
	*/
	
	auto& finish_trans = mainScene.GetComponent<TransformComponent>(finish_e.guid);
	TransformComponent& car_trans = mainScene.GetComponent<TransformComponent>(car_e.guid);
	


	FramerateCounter framerate;

	bool quit = false;
	int controlledCamera = 0;

	
  
	// GAME LOOP
	while (!quit) {
		Timestep timestep; // Time since last frame
		{
			uint32_t now_millisecs = SDL_GetTicks();
			uint32_t delta_millisecs = now_millisecs - lastTime_millisecs;
			if (delta_millisecs == 0) delta_millisecs = 1; // HACK: pretend at least one millisecond passes between each frame
			if (delta_millisecs > 200) delta_millisecs = 200; // HACK: clamp delta time between frames so physics doesn't go boom
			timestep = (float) delta_millisecs;
			lastTime_millisecs = now_millisecs;
		}


		//polls all pending input events until there are none left in the queue
		while (SDL_PollEvent(&window.event)) {
			ImGui_ImplSDL2_ProcessEvent(&window.event);

			if (window.event.type == SDL_CONTROLLERDEVICEADDED) {
				std::cout << "Adding controller\n";
				ControllerInput::init_controller();
			}

			if (window.event.type == SDL_CONTROLLERDEVICEREMOVED) {
				std::cout << "removing controller\n";
				ControllerInput::deinit_controller();
			}

			if (window.event.type == SDL_QUIT)
				quit = true;

			if (window.event.type == SDL_KEYDOWN) {
				switch (window.event.key.keysym.sym) {

					case SDLK_r:
						//TODO recompile the shader
						break;
						
					// TODO: change the file that is serializes (Want to do base.json and enginedrive.json)
					case SDLK_i:// i means in
						carConfig.deserialize();
						break;
					case SDLK_o:// o means out
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
					case SDLK_w:
						break;


					// Prinout of camera matrix
				case SDLK_c:
					std::cout << gs.getCameraView()[0][0] << ", " << gs.getCameraView()[0][1] << ", " << gs.getCameraView()[0][2] << ", " << gs.getCameraView()[0][3] << "," << std::endl;
					std::cout << gs.getCameraView()[1][0] << ", " << gs.getCameraView()[1][1] << ", " << gs.getCameraView()[1][2] << ", " << gs.getCameraView()[1][3] << "," << std::endl;
					std::cout << gs.getCameraView()[2][0] << ", " << gs.getCameraView()[2][1] << ", " << gs.getCameraView()[2][2] << ", " << gs.getCameraView()[2][3] << "," << std::endl;
					std::cout << gs.getCameraView()[3][0] << ", " << gs.getCameraView()[3][1] << ", " << gs.getCameraView()[3][2] << ", " << gs.getCameraView()[3][3] << std::endl;
					std::cout << std::endl;

					std::cout << finish_trans.getPosition().x << ", " << finish_trans.getPosition().y << ", " << finish_trans.getPosition().z << std::endl;
					std::cout << car_trans.getPosition().x << ", " << car_trans.getPosition().y << ", " << car_trans.getPosition().z << std::endl;
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

		// Finish line code
		if (car_trans.getPosition().x >= -1.5f && car_trans.getPosition().x <= 4.8f &&
			car_trans.getPosition().z >= -3.0f && car_trans.getPosition().z <= -0.6f)
		{
			if (isFinished == false) {
				isFinished = true;
				finishLinePrint();
			}

		}
		else {
			isFinished = false;
		}

		gs.Update(mainScene, 0.0f);
		aiSystem.Update(mainScene, 0.f);
		physicsSystem.Update(mainScene,timestep);


		// END__ ECS SYSTEMS UPDATES

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		// BEGIN FRAMERATE COUNTER
		framerate.update(timestep);
		ImGui::SetNextWindowSize(ImVec2(500, 100)); 
		ImGui::Begin("Milestone 2");
		ImGui::Text("framerate: %d", (int) framerate.framerate());
        ImGui::PlotLines("Frametime plot (ms)", framerate.m_time_queue_ms.data(), framerate.m_time_queue_ms.size());
        ImGui::PlotLines("Framerate plot (hz)", framerate.m_rate_queue.data(), framerate.m_rate_queue.size());
		// TODO(milestone 1): display physx value as proof that physx is initialized
		ImGui::End();
		// END FRAMERATE COUNTER

		// PHYSX DRIVER UPDATE
		// TODO(milestone 1): strip all non-milestone related imgui windows out
		// BEGIN CAR PHYSICS PANEL
		// ImGui::Begin("Car Physics", nullptr);
		// ImGui::SliderFloat("acceleration", &carPhysics.m_acceleration, 0.f, 1000.f);
		// ImGui::SliderFloat("suspension", &carPhysics.m_suspension_force, 0.f, 1000.f);
		// if (ImGui::Button("Serialize")) carConfig.serialize();
		// ImGui::End();


		// BEGIN A BUTTON THING
		/*bool cbutton = false;
		cbutton = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);
		ImGui::Begin("Buttons", nullptr);
		ImGui::Checkbox("a button", &cbutton);
		ImGui::End();*/
		// END A BUTTON THING

		// BEGIN JOYSTICK THING
		//float axis = 0;
		//axis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
		//ImGui::Begin("Axes aka Joysticks and triggers");
		//ImGui::Text("Right trigger: %hd", axis);
		//ImGui::End();
		// END JOYSTICK THING

		// HACK(beau): pull these out of CarSample.cpp
		extern float carThrottle;
		extern float carBrake;
		extern float carAxis;
		extern float carAxisScale;
		ImGui::Begin("Car commands tuner", nullptr);
		ImGui::Text("left stick horizontal tilt: %f", carAxis);
		ImGui::Text("Laps: %d", lapCount);
		ImGui::End();
		// END CAR PHYSICS PANEL

		// NOTE: the imgui bible - beau
		//ImGui::ShowDemoWindow();

		gs.ImGuiPanel();
		// Loads the imgui panel that lets you reload vehicle JSONs
		// reloadVehicleJSON();

		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.swapBuffers();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();


	// cleanupPhysics();
	physicsSystem.Cleanup();

	ControllerInput::deinit_controller();

	SDL_Quit();
	return 0;
}
