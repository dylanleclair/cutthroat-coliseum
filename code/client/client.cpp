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

#include "GRAPHICS_TESTBENCH.h"

#include "CarPhysics.h"
#include "FrameCounter.h"
#include "systems/ai.h"

#include "ImGuiTuneables.h"

#include "utils/Time.h"
#include "Input.h"

#include "systems/PhysicsSystem.h"

#include "Car.h"

#include "physics/LevelCollider.h"

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
bool tethered = false;

uint32_t lastTime_millisecs;

void finishLinePrint() {
	lapCount++;
	std::cout << "Lap: " << lapCount << std::endl;
	if (lapCount == 2) {
		std::cout << "You win !" << std::endl;
	}
}

// Provides a target (ideally the center of mass of a moving object)
// Renders the sphere where the transform for that object is
// This is the transform component version
void renderCMassSphere(TransformComponent &_target, TransformComponent& sphere_transform) {
	sphere_transform.setPosition(glm::vec3(_target.getTranslation().x, _target.getTranslation().y, _target.getTranslation().z));
}
// This is the PxTransform version as vehicle PhysX models use PxTransforms for their center of mass
void renderCMassSphere(PxTransform & _target, TransformComponent& sphere_transform) {
	sphere_transform.setPosition(glm::vec3(_target.p.x, _target.p.y, _target.p.z));
}

// Returns the x and y components of distances from the target object
// returns the magnitude / distance from to origin to target
std::vector<float> distFromTarget(TransformComponent _origin_object, TransformComponent _target_object) {
	std::vector<float> values;

	float x_diff = _origin_object.getTranslation().x - _target_object.getTranslation().x;
	float z_diff = _origin_object.getTranslation().z - _target_object.getTranslation().z;

	values.push_back(x_diff);
	values.push_back(z_diff);

	values.push_back(sqrt(pow(abs(x_diff), 2) + pow(abs(z_diff), 2)));

	return values;
}

// Is given an array, and returns the index of the smallest element in the array
// THE ARRAY SHOULD NOT BE SORTED
int findSmallestDistanceIndex(std::vector<float> _array) {
	float value = _array[0];
	int index = 0;
	for (int i = 0; i < _array.size(); i++) {
		float val_compare = _array[i];

		if (val_compare < value) {
			index = i;
		}
	}

	return index;
}

int main(int argc, char* argv[]) {
	//RUN_GRAPHICS_TEST_BENCH();
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
	ecs::Entity finish_e = mainScene.CreateEntity();
	ecs::Entity tetherPole1_e = mainScene.CreateEntity();
	ecs::Entity tetherPole2_e = mainScene.CreateEntity();
	ecs::Entity sphere_e = mainScene.CreateEntity();
	ecs::Entity tether_e = mainScene.CreateEntity();

	mainScene.AddComponent(car_e.guid, Car{});
	Car& testCar = mainScene.GetComponent<Car>(car_e.guid);
	testCar.physicsSystem = &physicsSystem;
	if (!testCar.initVehicle())
	{
		std::cout << "ERROR: could not initialize vehicle";
	}

	// Car Entity
	RenderModel car_r = RenderModel();
	GraphicsSystem::importOBJ(car_r, "alpha_cart.obj");
	car_r.setModelColor(glm::vec3(0.5f, 0.5f, 0.f));
	mainScene.AddComponent(car_e.guid, car_r);
	TransformComponent car_t = TransformComponent(testCar.getVehicleRigidBody());
	car_t.setPosition(glm::vec3(0, 0, 0.5f));
	//car_t.setRotation(glm::quat(0, 0, 0, 0));
	car_t.setScale(glm::vec3(3.2f, 3.2f, 3.2f));
	mainScene.AddComponent(car_e.guid, car_t);
	
	// Center of gravity sphere - used for debug
	RenderModel sphere_r = RenderModel();
	GraphicsSystem::importOBJ(sphere_r, "sphere.obj");
	sphere_r.setModelColor(glm::vec3(0.5f, 0.0f, 0.5f));
	mainScene.AddComponent(sphere_e.guid, sphere_r);
	//TransformComponent sphere_t = TransformComponent();
	TransformComponent sphere_t = TransformComponent(testCar.getVehicleRigidBody());
	sphere_t.setScale(glm::vec3(0.5f, 0.5f, 0.5f));
	mainScene.AddComponent(sphere_e.guid, sphere_t);



	//finish box
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
	GraphicsSystem::importOBJ(finish, "basic_finish.obj");
	//finish.attachMesh(finish_geom);
	finish.setModelColor(glm::vec3(1.f, 0.f, 0.f));
	mainScene.AddComponent(finish_e.guid, finish);

	TransformComponent finish_t = TransformComponent();
	finish_t.setPosition(glm::vec3(0, 0, 0));
	finish_t.setScale(glm::vec3(3.2f, 3.2f, 3.2f));
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
	level_t.setScale(glm::vec3(3.2f, 1.f, 3.2f));
	mainScene.AddComponent(ground_e.guid, level_t);

	// actual level mesh & collider for it
	LevelCollider levelCollider{"large_test_torus.obj", physicsSystem};
	auto levelTriangleMesh = levelCollider.cookLevel();
	levelCollider.initLevelRigidBody(levelTriangleMesh);

	RenderModel level_r = RenderModel();
	GraphicsSystem::importOBJ(level_r, "large_test_torus.obj");
	level_r.setModelColor(glm::vec3(0, 0, 1));
	mainScene.AddComponent(level_e.guid, level_r);
	mainScene.AddComponent(level_e.guid, level_t);

	TransformComponent wall_t = TransformComponent();
	wall_t.setPosition(glm::vec3(0, 0, 0));
	wall_t.setScale(glm::vec3(3.2f, 3.2f, 3.2f));

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

	// Tether poles
	RenderModel tetherPole1_r = RenderModel();
	GraphicsSystem::importOBJ(tetherPole1_r, "alpha_tether_pole.obj");
	tetherPole1_r.setModelColor(glm::vec3(205.f / 255.f, 133.f / 255.f, 63.f / 255.f));
	mainScene.AddComponent(tetherPole1_e.guid, tetherPole1_r);
	TransformComponent tetherPole1_t = TransformComponent();
	tetherPole1_t.setPosition(glm::vec3(-27.f, 0.f, 50.f));
	tetherPole1_t.setScale(glm::vec3(3.2f, 3.2f, 3.2f));
	mainScene.AddComponent(tetherPole1_e.guid, tetherPole1_t);

	RenderModel tetherPole2_r = RenderModel();
	GraphicsSystem::importOBJ(tetherPole2_r, "alpha_tether_pole.obj");
	tetherPole2_r.setModelColor(glm::vec3(205.f / 255.f, 133.f / 255.f, 63.f / 255.f));
	mainScene.AddComponent(tetherPole2_e.guid, tetherPole2_r);
	TransformComponent tetherPole2_t = TransformComponent();
	tetherPole2_t.setPosition(glm::vec3(-27.f, 0.f, -50.f));
	tetherPole2_t.setScale(glm::vec3(3.2f, 3.2f, 3.2f));
	mainScene.AddComponent(tetherPole2_e.guid, tetherPole2_t);
	
	// Tether
	RenderModel tether_r = RenderModel();
	GraphicsSystem::importOBJ(tether_r, "alpha_tether.obj");
	tether_r.setModelColor(glm::vec3(83.f / 255.f, 54.f / 255.f, 33.f / 255.f));
	mainScene.AddComponent(tether_e.guid, tether_r);
	TransformComponent tether_t = TransformComponent();
	tether_t.setPosition(glm::vec3(- 27.f, 1.f, 45.f));
	tether_t.setScale(glm::vec3(1.f, 2.f, 2.f));
	mainScene.AddComponent(tether_e.guid, tether_t);

	// This is how to change the position of the object after it has been passed to the ECS
	/*
	auto &wallTrans = mainScene.GetComponent<TransformComponent>(outWall_e.guid);
	wallTrans.setPosition(glm::vec3(0, 0, 0));
	*/
	
	// Fetching ecs components for system call debugs and other
	// debug functionality
	auto &finish_trans = mainScene.GetComponent<TransformComponent>(finish_e.guid);
	TransformComponent &car_trans = mainScene.GetComponent<TransformComponent>(car_e.guid);
	TransformComponent &sphere_transform = mainScene.GetComponent<TransformComponent>(sphere_e.guid);
	TransformComponent &tetherPole1_transform = mainScene.GetComponent<TransformComponent>(tetherPole1_e.guid);
	TransformComponent &tetherPole2_transform = mainScene.GetComponent<TransformComponent>(tetherPole2_e.guid);
	TransformComponent &tether_transform = mainScene.GetComponent<TransformComponent>(tether_e.guid);
	PxTransform loc;

	std::vector<TransformComponent> tethers;
	tethers.push_back(tetherPole1_transform);
	tethers.push_back(tetherPole2_transform);

	FramerateCounter framerate;

	bool quit = false;
	int controlledCamera = 0;
	
	// Initalizes variables for the vehicle tuning Imgui
	baseVariablesInit(testCar.m_Vehicle);
	engineVariablesInit(testCar.m_Vehicle);
  
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
						// Rudementary car reset (will keep using the velocity and rotation of the car through the rest)
						testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->setGlobalPose(PxTransform(0.f, 0.f, 0.f));
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
					case SDLK_SPACE:
						testCar.TetherJump();
						break;
					case SDLK_m:
						if (!tethered) {
							tethered = true;
							loc.p.x = tetherPole1_transform.getTranslation().x;
							loc.p.y = tetherPole1_transform.getTranslation().y;
							loc.p.z = tetherPole1_transform.getTranslation().z;
							testCar.TetherSteer(loc);
						}
						else if (tethered) {
							tethered = false;
							testCar.resetModifications();
						}

						break;


					// Prinout of camera matrix
				case SDLK_c:
					std::cout << gs.getCameraView()[0][0] << ", " << gs.getCameraView()[0][1] << ", " << gs.getCameraView()[0][2] << ", " << gs.getCameraView()[0][3] << "," << std::endl;
					std::cout << gs.getCameraView()[1][0] << ", " << gs.getCameraView()[1][1] << ", " << gs.getCameraView()[1][2] << ", " << gs.getCameraView()[1][3] << "," << std::endl;
					std::cout << gs.getCameraView()[2][0] << ", " << gs.getCameraView()[2][1] << ", " << gs.getCameraView()[2][2] << ", " << gs.getCameraView()[2][3] << "," << std::endl;
					std::cout << gs.getCameraView()[3][0] << ", " << gs.getCameraView()[3][1] << ", " << gs.getCameraView()[3][2] << ", " << gs.getCameraView()[3][3] << std::endl;
					std::cout << std::endl;

					std::cout << "finish line: " << finish_trans.getTranslation().x << ", " << finish_trans.getTranslation().y << ", " << finish_trans.getTranslation().z << std::endl;
					std::cout << std::endl;

					std::cout << "tether pole 1: " << tetherPole1_t.getTranslation().x << ", " << tetherPole1_t.getTranslation().y << "," << tetherPole1_t.getTranslation().z << std::endl;
					std::cout << std::endl;

					std::cout << "Car Transform: " << std::endl;

					std::cout << car_trans.getTranslation().x << ", " << car_trans.getTranslation().y << ", " << car_trans.getTranslation().z << std::endl;


					std::cout << mainScene.GetComponent<TransformComponent>(car_e.guid).getTranslation().x << ","
						<< mainScene.GetComponent<TransformComponent>(car_e.guid).getTranslation().y << ","
						<< mainScene.GetComponent<TransformComponent>(car_e.guid).getTranslation().z << std::endl;

					std::cout << testCar.getVehicleRigidBody()->getGlobalPose().p.x << ", "
							  << testCar.getVehicleRigidBody()->getGlobalPose().p.y << ", "
						      << testCar.getVehicleRigidBody()->getGlobalPose().p.z << std::endl;
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

		// Check for the car grounded state, and if grounded after being in the air
		// resets the modifications made to the car while in the air
		// This is because angular dampening is applied while in the air to prevent
		// the car from flipping, but on the ground it can't turn so it resets
		// the angular dampening on the ground
		if (testCar.isGroundedDelay(testCar)) {
			testCar.resetModifications();
		}
		PxTransform c_mass_f;
		//c_mass_f.p.x = car_trans.getTranslation().x - tetherPole1_t.getTranslation().x;
		//c_mass_f.p.y = car_trans.getTranslation().y - tetherPole1_t.getTranslation().y;
		//c_mass_f.p.z = car_trans.getTranslation().z - tetherPole1_t.getTranslation().z;
		//c_mass_f.p.x = tetherPole1_t.getTranslation().x;
		//c_mass_f.p.y = tetherPole1_t.getTranslation().y;
		//c_mass_f.p.z = tetherPole1_t.getTranslation().z;
		//testCar.m_Vehicle.mPhysXParams.physxActorCMassLocalPose = c_mass_f;
		

		auto& center_of_mass = testCar.m_Vehicle.mPhysXParams.physxActorCMassLocalPose;
		renderCMassSphere(center_of_mass, sphere_transform);

		if (tethered) {
			std::vector<float> p1_distances = distFromTarget(car_trans, tetherPole1_transform);
			std::vector<float> p2_distances = distFromTarget(car_trans, tetherPole2_transform);
			std::vector<float> p_distances;
			p_distances.clear();
			p_distances.push_back(p1_distances[2]);
			p_distances.push_back(p2_distances[2]);

			testCar.setClosestTetherPoint(tethers[findSmallestDistanceIndex(p_distances)].getTranslation());

			std::vector<float> distances = distFromTarget(car_trans, tethers[findSmallestDistanceIndex(p_distances)]);
			//float x_diff = car_trans.getTranslation().x - tetherPole1_transform.getTranslation().x;
			//float z_diff = car_trans.getTranslation().z - tetherPole1_transform.getTranslation().z;
			float tether_angle = atan(distances[0] / distances[1]);
			// Puts the tether's origin at the tether point
			tether_transform.setPosition(glm::vec3(tethers[findSmallestDistanceIndex(p_distances)].getTranslation().x, 1.5f, tethers[findSmallestDistanceIndex(p_distances)].getTranslation().z));
			// Sets the scale of the tether on the x axis in accordance to the distance of the car to the tether point
			tether_transform.setScale(glm::vec3(distances[2], 1.f, 1.f));
			// Rotates the tether to track the car
			if (distances[1] < 0) {
				tether_transform.setRotation(glm::vec3(0, 1, 0), tether_angle + (M_PI / 2.f));
			}
			else {
				tether_transform.setRotation(glm::vec3(0, 1, 0), tether_angle + (3 * M_PI / 2.f));
			}
		}
		else {
			tether_transform.setScale(glm::vec3(0.f, 0.f, 0.f));
		}

		// Finish line code
		if (car_trans.getTranslation().x >= -1.5f && car_trans.getTranslation().x <= 4.8f &&
			car_trans.getTranslation().z >= -3.0f && car_trans.getTranslation().z <= -0.6f)
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
		ImGui::Begin("Milestone 3");
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
		extern float controller_throttle;
		extern float controller_brake;
		extern float carAxis;
		extern float carAxisScale;
		ImGui::Begin("Car commands tuner", nullptr);
		ImGui::Text("left stick horizontal tilt: %f", carAxis);
		//ImGui::Text("Car Throttle: %f", controller_throttle);
		//ImGui::Text("Car Brake: %f", controller_brake);

		ImGui::Text("Current Gear: %d", testCar.m_Vehicle.mEngineDriveState.gearboxState.currentGear);
		ImGui::Text("Current engine rotational speed: %f", testCar.m_Vehicle.mEngineDriveState.engineState.rotationSpeed);
		ImGui::Text("Center of Gravity: %f, %f, %f", testCar.m_Vehicle.mPhysXParams.physxActorCMassLocalPose.p.x, 
													testCar.m_Vehicle.mPhysXParams.physxActorCMassLocalPose.p.y, 
													testCar.m_Vehicle.mPhysXParams.physxActorCMassLocalPose.p.z);
		ImGui::Text("Suspension force x: %f", testCar.m_Vehicle.mBaseState.suspensionForces->force.x);
		ImGui::Text("Suspension force y: %f", testCar.m_Vehicle.mBaseState.suspensionForces->force.y);
		ImGui::Text("Suspension force z: %f", testCar.m_Vehicle.mBaseState.suspensionForces->force.z);
		ImGui::Text("On the ground ?: %s", testCar.m_Vehicle.mBaseState.roadGeomStates->hitState ? "true" : "false");
		ImGui::Text("Laps: %d", lapCount);
		ImGui::End();
		// END CAR PHYSICS PANEL

		// NOTE: the imgui bible - beau
		//ImGui::ShowDemoWindow();

		// Graphics imgui panel for graphics tuneables
		gs.ImGuiPanel();
	
		//ImGui Panels for tuning
		//reloadVehicleJSON();
		vehicleTuning(testCar.m_Vehicle);
		engineTuning(testCar.m_Vehicle);


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
