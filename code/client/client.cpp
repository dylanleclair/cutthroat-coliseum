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


using namespace physx;

extern PxRigidBody* getVehicleRigidBody();
extern bool initPhysics();
extern void stepPhysics(SDL_GameController* controller, float timestep);
extern void cleanupPhysics();
extern int carSampleInit();

extern PxScene* gScene;

CarPhysics carPhysics;
CarPhysicsSerde carConfig(carPhysics);

int lapCount = 0;

void finishLineLogic() {
	lapCount++;
	std::cout << "Lap: " << lapCount << std::endl;
	if (lapCount == 2) {
		std::cout << "You win !" << std::endl;
	}
	
}


int main(int argc, char* argv[]) {
	printf("Starting main");


	carSampleInit();

	SDL_Init(SDL_INIT_EVERYTHING); // initialize all sdl systems
	Window window(1200, 800, "Maximus Overdrive");

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
	CPU_Geometry ground_geom;
	glm::vec3 square[] = {
		{1.f, 0.f, 1.0f},
		{1.f, 0.f, -1.0f},
		{-1.f, 0.f, -1.0f},

		{1.f, 0.f, 1.0f},
		{-1.f, 0.f, -1.0f},
		{-1.f, 0.f, 1.0f},
	};

	const float scale = 1.f;
	for (int x = -10; x <= 10; x++) {
		for (int z = -10; z <= 10; z++) {
			for (int i = 0; i < 6; i++) {
				ground_geom.verts.push_back(square[i]*scale + glm::vec3(x * 2 * scale,-1,z * 2 * scale));
				glm::vec3 color = (float)((abs(x) + abs(z)) % 2) * glm::vec3(0.65, 0, .95);
				ground_geom.cols.push_back(glm::vec3(color));
				ground_geom.norms.push_back(glm::vec3(0, 1, 0));
			}
		}
	}


	
	//make an entity
	ecs::Entity car_e = mainScene.CreateEntity();
	ecs::Entity level_e = mainScene.CreateEntity();
	ecs::Entity outWall_e = mainScene.CreateEntity();
	ecs::Entity inWall_e = mainScene.CreateEntity();
	ecs::Entity ground_e = mainScene.CreateEntity();

	// Ground plane component
	RenderComponent ground = RenderComponent(&ground_geom);
	ground.shaderState |= 4;
	mainScene.AddComponent(ground_e.guid, ground);

	TransformComponent trans2 = TransformComponent();
	mainScene.AddComponent(ground_e.guid, trans2);


	// Car
	RenderComponent car_r = RenderComponent();
	GraphicsSystem::readVertsFromFile(car_r, "models/test_car.obj");
	car_r.shaderState |= 4; // 4 is for shading
	car_r.shaderState |= 1; // 1 is for colours
	car_r.color = glm::vec3(0.5f, 0.5f, 0.f);
	mainScene.AddComponent(car_e.guid, car_r);
	TransformComponent car_t = TransformComponent(getVehicleRigidBody());
	car_t.setPosition(glm::vec3(0, 1, 0));
	mainScene.AddComponent(car_e.guid, car_t);

	auto& car_render = mainScene.GetComponent<RenderComponent>(car_e.guid);
	std::cout << "Car Guid: " << car_e.guid << std::endl;
	car_render.appearance = 0;


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
		finish_geom.cols.push_back(glm::vec3(1, 0, 0));
	}
	for (int i = 0; i < 6; i++) {
		finish_geom.verts.push_back(rectangle[5-i]);
		finish_geom.cols.push_back(glm::vec3(1, 0, 0));
	}


	// Finish line components
	RenderComponent finish = RenderComponent(&finish_geom);
	finish.appearance = 0;
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
	mainScene.AddComponent(path.guid,RenderComponent{});

	AISystem aiSystem{path.guid};


	// Level
	TransformComponent level_t = TransformComponent();
	mainScene.AddComponent(ground_e.guid, level_t);

	RenderComponent level_r = RenderComponent();
	GraphicsSystem::readVertsFromFile(level_r, "models/large_test_torus.obj");
	level_r.shaderState |= 1;
	level_r.shaderState |= 4;
	level_r.color = glm::vec3(0.f, 0.f, 1.f);
	mainScene.AddComponent(level_e.guid, level_r);	
	mainScene.AddComponent(level_e.guid, level_t);

	TransformComponent wall_t = TransformComponent();
	wall_t.setPosition(glm::vec3(0, 2, 0));

	RenderComponent outWall = RenderComponent();
	GraphicsSystem::readVertsFromFile(outWall, "models/large_test_torus_inwall.obj");
	outWall.shaderState |= 1;
	outWall.shaderState |= 4;
	outWall.color = glm::vec3(0.2f, 0.2f, 0.6f);
	mainScene.AddComponent(outWall_e.guid, outWall);
	mainScene.AddComponent(outWall_e.guid, wall_t);

	RenderComponent inWall = RenderComponent();
	GraphicsSystem::readVertsFromFile(inWall, "models/large_test_torus_outwall.obj");
	inWall.shaderState |= 1;
	inWall.shaderState |= 4;
	inWall.color = glm::vec3(0.2f, 0.2f, 0.6f);
	mainScene.AddComponent(inWall_e.guid, inWall);
	mainScene.AddComponent(inWall_e.guid, wall_t);

	// This is how to change the position of the object after it has been passed to the ECS
	/*
	auto &wallTrans = mainScene.GetComponent<TransformComponent>(outWall_e.guid);
	wallTrans.setPosition(glm::vec3(0, 0, 0));
	*/
	auto &finish_trans = mainScene.GetComponent<TransformComponent>(finish_e.guid);
	TransformComponent &car_trans = mainScene.GetComponent<TransformComponent>(car_e.guid);
	


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
						std::cout << gs.getCameraView()[0][0] << ", " << gs.getCameraView()[0][1] << ", " << gs.getCameraView()[0][2] << ", " << gs.getCameraView()[0][3] << "," << std::endl;
						std::cout << gs.getCameraView()[1][0] << ", " << gs.getCameraView()[1][1] << ", " << gs.getCameraView()[1][2] << ", " << gs.getCameraView()[1][3] << "," << std::endl;
						std::cout << gs.getCameraView()[2][0] << ", " << gs.getCameraView()[2][1] << ", " << gs.getCameraView()[2][2] << ", " << gs.getCameraView()[2][3] << "," << std::endl;
						std::cout << gs.getCameraView()[3][0] << ", " << gs.getCameraView()[3][1] << ", " << gs.getCameraView()[3][2] << ", " << gs.getCameraView()[3][3] << std::endl;
						std::cout << std::endl;
						
						std::cout << finish_trans.getPosition().x << ", " << finish_trans.getPosition().y << ", " << finish_trans.getPosition().z << std::endl;
						std::cout << car_t.getPosition().x << ", " << car_t.getPosition().y << ", " << car_t.getPosition().z << std::endl;
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
		if (car_trans.getPosition().x >= -0.5f && car_trans.getPosition().x <= 3.8f &&
			car_trans.getPosition().z >= -1.9f && car_trans.getPosition().z <= -1.85f)
		{
			finishLineLogic();
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
		aiSystem.Update(mainScene, 0.f);

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
		stepPhysics(controller, framerate.m_time_queue.front() / 1000.f);


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
