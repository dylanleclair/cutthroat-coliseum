#include <iostream>


#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "systems/graphics/Geometry.h"
#include "PxPhysicsAPI.h"

#include "Window.h"

#include "core/ecs.h"
#include "systems/GraphicsSystem.h"
#include "systems/components.h"

#include "GRAPHICS_TESTBENCH.h"

#include "FrameCounter.h"
#include "systems/ai.h"

#include "ImGuiTuneables.h"

#include "utils/Time.h"
#include "Input.h"

#include "systems/PhysicsSystem.h"

#include "entities/car/Car.h"
#include "entities/car/AICar.h"
#include "entities/car/AIEntity.h"


#include "TetherGraphics.h"
#include "entities/physics/Obstacles.h"

#include "entities/physics/LevelCollider.h"

#include "systems/SoundSystem.h"

glm::vec3 calculateSpherePoint(float s, float t)
{
	float z = cos(2 * M_PI * t) * sin(M_PI * s);
	float x = sin(2 * M_PI * t) * sin(M_PI * s);
	float y = cos(M_PI * s);
	return(glm::vec3(x, y, z));
}

// CarPhysics carPhysics;
// CarPhysicsSerde carConfig(carPhysics);

bool showImgui = true;

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

int main(int argc, char* argv[]) {
	//RUN_GRAPHICS_TEST_BENCH();
	printf("Starting main");



	init_sound_system();


	SDL_Init(SDL_INIT_EVERYTHING); // initialize all sdl systems
	Window window(1200, 800, "Maximus Overdrive");

	lastTime_millisecs = SDL_GetTicks();

	ImGuiIO& io = ImGui::GetIO(); (void)io;

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


	//load fonts into ImGui
	io.Fonts->AddFontDefault();
	ImFont* Debrosee = io.Fonts->AddFontFromFileTTF("fonts/Debrosee-ALPnL.ttf", 18.5f);
	IM_ASSERT(Debrosee != NULL);
	ImFont* Cabal = io.Fonts->AddFontFromFileTTF("fonts/Cabal-w5j3.ttf", 18.5f);
	IM_ASSERT(Cabal != NULL);
	ImFont* CabalBold = io.Fonts->AddFontFromFileTTF("fonts/CabalBold-78yP.ttf", 32.f);
	IM_ASSERT(CabalBold != NULL);
	ImFont* ExtraLarge = io.Fonts->AddFontFromFileTTF("fonts/EXTRA LARGE.ttf", 18.5f);
	IM_ASSERT(ExtraLarge != NULL);


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

	// ecs::Entity aiDriver_e = mainScene.CreateEntity();

	mainScene.AddComponent(car_e.guid, Car{});
	Car& testCar = mainScene.GetComponent<Car>(car_e.guid);
	testCar.physicsSystem = &physicsSystem;
	if (!testCar.initVehicle(PxVec3(35.000000000f, -0.0500000119f, -1.59399998f)))
	{
		std::cout << "ERROR: could not initialize vehicle";
	}

	NavPath circlePath = generateCirclePath(30);

	CPU_Geometry aiPathGeom;
	// import the obj for the path
	GraphicsSystem::importOBJ(aiPathGeom, "ai_path.obj");
	// transform verts same way level will be???


	auto scaling =  glm::scale(glm::mat4{1.f},glm::vec3(3.2f, 3.2f, 3.2f));
	for (auto& vert : aiPathGeom.verts)
	{
			vert = scaling * glm::vec4{vert,1.f}; 
			vert.y = 0.f;
	}

	NavPath aiPath{aiPathGeom.verts};

	ecs::Entity navRenderer_e = mainScene.CreateEntity();
	mainScene.AddComponent(navRenderer_e.guid,TransformComponent{});
	auto navPathRender = RenderLine{aiPathGeom};
	navPathRender.setColor(glm::vec3{1.0f,0.f,1.0f});
	mainScene.AddComponent(navRenderer_e.guid,navPathRender);

	// only spawn one for now!! consider this ur final warning.
	//spawnAIEntity(mainScene,&physicsSystem, car_e.guid,{10.f, 10.f,10.f}, &aiPath);
	Guid aiCarGuid = spawnAIEntity(mainScene, &physicsSystem, car_e.guid, { 10.f, 10.f,10.f }, &aiPath);
	AICar& aiCarInstance = mainScene.GetComponent<AICar>(aiCarGuid);
	// spawnAIEntity(mainScene,&physicsSystem, car_e.guid,{0.f, 0.f,5.f}, &circlePath);
	

	// Car Entity
	RenderModel car_r = RenderModel();
	GraphicsSystem::importOBJ(car_r, "alpha_cart.obj");
	car_r.setModelColor(glm::vec3(0.5f, 0.5f, 0.f));
	car_r.isShadowed(true);
	mainScene.AddComponent(car_e.guid, car_r);
	TransformComponent car_t = TransformComponent(testCar.getVehicleRigidBody());
	car_t.setPosition(glm::vec3(0, 0, 0.5f));
	car_t.setScale(glm::vec3(3.2f, 3.2f, 3.2f));
	mainScene.AddComponent(car_e.guid, car_t);
	
	// Center of gravity sphere - used for debug
	RenderModel sphere_r = RenderModel();
	GraphicsSystem::importOBJ(sphere_r, "sphere.obj");
	sphere_r.setModelColor(glm::vec3(0.5f, 0.0f, 0.5f));
	mainScene.AddComponent(sphere_e.guid, sphere_r);
	TransformComponent sphere_t = TransformComponent(testCar.getVehicleRigidBody());
	sphere_t.setScale(glm::vec3(0.5f, 0.5f, 0.5f));
	mainScene.AddComponent(sphere_e.guid, sphere_t);

	
	// Finish line components
	RenderModel finish = RenderModel();
	GraphicsSystem::importOBJ(finish, "basic_finish.obj");
	finish.setModelColor(glm::vec3(1.f, 0.f, 0.f));
	mainScene.AddComponent(finish_e.guid, finish);

	TransformComponent finish_t = TransformComponent();
	finish_t.setPosition(glm::vec3(30, 0, 0));
	finish_t.setScale(glm::vec3(3.2f, 3.2f, 3.2f));
	mainScene.AddComponent(finish_e.guid, finish_t);

	// Pathfinding
	// PathfindingComponent car_pathfinder{ finish_e.guid };
	// mainScene.AddComponent(car_e.guid, car_pathfinder);
	

	// Path renderer
	ecs::Entity path = mainScene.CreateEntity();
	mainScene.AddComponent(path.guid, TransformComponent{});
	mainScene.AddComponent(path.guid, RenderLine{});

	AISystem aiSystem{ path.guid };

	
	// Level
	TransformComponent level_t = TransformComponent();
	level_t.setScale(glm::vec3(3.2f, 3.2f, 3.2f));
	mainScene.AddComponent(ground_e.guid, level_t);

	// actual level mesh & collider for it
	CPU_Geometry levelColliderFloor_raw = CPU_Geometry();
	GraphicsSystem::importOBJ(levelColliderFloor_raw, "STADIUM_COLLIDER_FLOOR.obj");
	LevelCollider levelCollider1{ levelColliderFloor_raw, physicsSystem};
	auto levelTriangleMesh1 = levelCollider1.cookLevel(glm::scale(glm::mat4(1), glm::vec3(3.2)));
	levelCollider1.initLevelRigidBody(levelTriangleMesh1);

	CPU_Geometry levelColliderInner_raw = CPU_Geometry();
	GraphicsSystem::importOBJ(levelColliderInner_raw, "STADIUM_COLLIDER_INNER.obj");
	LevelCollider levelCollider2{ levelColliderInner_raw, physicsSystem };
	auto levelTriangleMesh2 = levelCollider2.cookLevel(glm::scale(glm::mat4(1), glm::vec3(3.2)));
	levelCollider2.initLevelRigidBody(levelTriangleMesh2);

	CPU_Geometry levelColliderOuter_raw = CPU_Geometry();
	GraphicsSystem::importOBJ(levelColliderOuter_raw, "STADIUM_COLLIDER_OUTER.obj");
	LevelCollider levelCollider3{ levelColliderOuter_raw, physicsSystem };
	auto levelTriangleMesh3 = levelCollider3.cookLevel(glm::scale(glm::mat4(1), glm::vec3(3.2)));
	levelCollider3.initLevelRigidBody(levelTriangleMesh3);

	//Level
	RenderModel level_r = RenderModel();
	//GraphicsSystem::importOBJ(level_r, "Stadium.obj");
	GraphicsSystem::importOBJ(level_r, "Stadium_MINIMAL.obj"); //for faster loading times
	mainScene.AddComponent(level_e.guid, level_r);
	mainScene.AddComponent(level_e.guid, level_t);

	// Tether poles
	RenderModel tetherPole1_r = RenderModel();
	GraphicsSystem::importOBJ(tetherPole1_r, "alpha_tether_pole.obj");
	tetherPole1_r.setModelColor(glm::vec3(205.f / 255.f, 133.f / 255.f, 63.f / 255.f));
	mainScene.AddComponent(tetherPole1_e.guid, tetherPole1_r);
	TransformComponent tetherPole1_t = TransformComponent();
	tetherPole1_t.setPosition(glm::vec3(0.f, 0.f, 162.f));
	tetherPole1_t.setScale(glm::vec3(3.2f, 3.2f, 3.2f));
	mainScene.AddComponent(tetherPole1_e.guid, tetherPole1_t);
	
	RenderModel tetherPole2_r = RenderModel();
	GraphicsSystem::importOBJ(tetherPole2_r, "alpha_tether_pole.obj");
	tetherPole2_r.setModelColor(glm::vec3(205.f / 255.f, 133.f / 255.f, 63.f / 255.f));
	mainScene.AddComponent(tetherPole2_e.guid, tetherPole2_r);
	TransformComponent tetherPole2_t = TransformComponent();
	tetherPole2_t.setPosition(glm::vec3(0.f, 0.f, -162.f));
	tetherPole2_t.setScale(glm::vec3(3.2f, 3.2f, 3.2f));
	mainScene.AddComponent(tetherPole2_e.guid, tetherPole2_t);
	
	// Tether
	RenderModel tether_r = RenderModel();
	GraphicsSystem::importOBJ(tether_r, "alpha_tether.obj");
	tether_r.setModelColor(glm::vec3(83.f / 255.f, 54.f / 255.f, 33.f / 255.f));
	mainScene.AddComponent(tether_e.guid, tether_r);
	TransformComponent tether_t = TransformComponent();
	tether_t.setPosition(glm::vec3(0.f, 1.f, 0.f));
	tether_t.setScale(glm::vec3(1.f, 2.f, 2.f));
	mainScene.AddComponent(tether_e.guid, tether_t);

	/*
	* Demonstration of the Billboard Component. It always expects a texture to be used and an optinal locking axis can be used
	* The Billboard will always try to face the camera
	*/
	ecs::Entity billboard = mainScene.CreateEntity();
	BillboardComponent bill_r = BillboardComponent("textures/CFHX3384.JPG", glm::vec3(0,1,0));
	TransformComponent bill_t = TransformComponent();
	bill_t.setPosition(glm::vec3(0, 20, 0));
	bill_t.setScale(glm::vec3(10, 5, 0));
	mainScene.AddComponent(billboard.guid, bill_r);
	mainScene.AddComponent(billboard.guid, bill_t);


	// Setting up log obstacles (currently boxes)
	setUpLogs(mainScene);
	addRigidBody(physicsSystem);

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

	std::vector<TransformComponent> c_tether_points;
	c_tether_points.push_back(tetherPole1_transform);
	c_tether_points.push_back(tetherPole2_transform);

	FramerateCounter framerate;

	bool quit = false;
	int controlledCamera = 0;
	
	// Initalizes variables for the vehicle tuning Imgui
	baseVariablesInit(testCar.m_Vehicle);
	engineVariablesInit(testCar.m_Vehicle);


	bool playSounds = true;

	// Find the default values of movement dampening
	// This will be used to reset changes to dampening
	auto default_lin_damp = testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->getLinearDamping();
	auto default_ang_damp = testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->getAngularDamping();

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

		// Reset dampening values if they are changed every frame (used after car is reset) 
		//TODO:  May need to put in an if check, and factor out ?
		testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->setLinearDamping(default_lin_damp);
		testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->setAngularDamping(default_ang_damp);

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
						// Rudementary car reset (will keep using the velocity and rotation of the car through the rest).
						testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->setGlobalPose(PxTransform(35.f, 0.f, 0.f));
						testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->setLinearDamping(10000.f);
						testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->setAngularDamping(10000.f);
						lapCount = 1;
						aiCarInstance.m_lapCount = 1;
						// TODO: apply the dampening to ai when resetting the ai
						// Will need to for loop all ai cars
						aiCarInstance.m_Vehicle.mPhysXState.physxActor.rigidBody->setGlobalPose(PxTransform(10.f, 2.f, 10.f));
						break;
						
					// TODO: change the file that is serializes (Want to do base.json and enginedrive.json)
					case SDLK_i:// i means in
						break;
					case SDLK_o:// o means out
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
					case SDLK_p:
						if (!showImgui) {
							showImgui = true;
						}
						else if (showImgui) {
							showImgui = false;
						}
						break;
					case SDLK_SPACE:
						testCar.TetherJump();
						break;
					case SDLK_m:
						if (!testCar.getCTethered()) {
							loc.p.x = tetherPole1_transform.getTranslation().x;
							loc.p.y = tetherPole1_transform.getTranslation().y;
							loc.p.z = tetherPole1_transform.getTranslation().z;
							testCar.TetherSteer(loc);
						}
						else if (testCar.getCTethered()) {
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
				case SDLK_5: {
					playSounds = !playSounds;
					break;
				}
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

		// Debug stuff for centre of mass - not working properly
		//c_mass_f.p.x = car_trans.getTranslation().x - tetherPole1_t.getTranslation().x;
		//c_mass_f.p.y = car_trans.getTranslation().y - tetherPole1_t.getTranslation().y;
		//c_mass_f.p.z = car_trans.getTranslation().z - tetherPole1_t.getTranslation().z;
		//c_mass_f.p.x = tetherPole1_t.getTranslation().x;
		//c_mass_f.p.y = tetherPole1_t.getTranslation().y;
		//c_mass_f.p.z = tetherPole1_t.getTranslation().z;
		//testCar.m_Vehicle.mPhysXParams.physxActorCMassLocalPose = c_mass_f;
		

		auto& center_of_mass = testCar.m_Vehicle.mPhysXParams.physxActorCMassLocalPose;
		renderCMassSphere(center_of_mass, sphere_transform);

		// Tether check to render the tether graphic properly
		if (testCar.getCTethered()) {
			updateTetherGraphic(car_trans, c_tether_points, testCar, tether_transform);
		}
		else {
			tether_transform.setScale(glm::vec3(0.f, 0.f, 0.f));
		}

		// Finish line code
		if (car_trans.getTranslation().x >= 28.f && car_trans.getTranslation().x <= 40.f &&
			car_trans.getTranslation().z >= -2.f && car_trans.getTranslation().z <= 0.f)
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

		update_sounds(testCar, aiCarInstance, playSounds);

		// END__ ECS SYSTEMS UPDATES

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		if (showImgui) {
			// BEGIN FRAMERATE COUNTER
			framerate.update(timestep);
			ImGui::SetNextWindowSize(ImVec2(500, 100));
			ImGui::Begin("Milestone 3");
			ImGui::Text("framerate: %d", (int)framerate.framerate());
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
			ImGui::Text("Car Location: %f, %f", car_trans.getTranslation().x, car_trans.getTranslation().z);
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
		}
		/*
		* Render the UI. I am doing this here for now but I might move it.
		* ImGui uses coordinates in screen space (0-screen dimension) and is anchored on the top left corner
		*/
		//render the UI
		// Setting flags
		ImGuiWindowFlags textWindowFlags =
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoMove |				// text "window" should not move
			ImGuiWindowFlags_NoResize |				// should not resize
			ImGuiWindowFlags_NoCollapse |			// should not collapse
			ImGuiWindowFlags_NoSavedSettings |		// don't want saved settings mucking things up
			ImGuiWindowFlags_AlwaysAutoResize |		// window should auto-resize to fit the text
			ImGuiWindowFlags_NoBackground |			// window should be transparent; only the text should be visible
			ImGuiWindowFlags_NoDecoration |			// no decoration; only the text should be visible
			ImGuiWindowFlags_NoTitleBar;			// no title; only the text should be visible

		//Lap counter
		ImGui::SetNextWindowPos(ImVec2(10, 10));
		ImGui::Begin("UI", (bool*)0, textWindowFlags);
		ImGui::SetWindowFontScale(2.f);
		ImGui::PushFont(CabalBold);
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Lap: %d/3", lapCount);
		ImGui::PopFont();
		ImGui::End();

		//Lap counter
		ImGui::SetNextWindowPos(ImVec2(10, 20));
		ImGui::Begin("UI", (bool*)0, textWindowFlags);
		ImGui::SetWindowFontScale(2.f);
		ImGui::PushFont(CabalBold);
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "AI Lap: %d/3", aiCarInstance.m_lapCount);
		ImGui::PopFont();
		ImGui::End();

		

		//you win message
		static int counter = 0;
		const float delayInSeconds = 0.5;
		static bool display = true;
		if (lapCount >= 3) {
			counter += timestep.getMilliseconds();
			if (counter >= delayInSeconds * 1000) {
				counter = 0;
				display = !display;
			}
			if (display) {
				ImGui::SetNextWindowPos(ImVec2(200, 200));
				ImGui::Begin("UI2", (bool*)0, textWindowFlags);
				ImGui::SetWindowFontScale(5.f);
				ImGui::PushFont(CabalBold);
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "VICTORY");
				ImGui::PopFont();
				ImGui::End();
			}
		}

		else if (aiCarInstance.m_lapCount >= 3) {
			counter += timestep.getMilliseconds();
			if (counter >= delayInSeconds * 1000) {
				counter = 0;
				display = !display;
			}
			if (display) {
				ImGui::SetNextWindowPos(ImVec2(200, 200));
				ImGui::Begin("UI2", (bool*)0, textWindowFlags);
				ImGui::SetWindowFontScale(5.f);
				ImGui::PushFont(CabalBold);
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "AI VICTORY");
				ImGui::PopFont();
				ImGui::End();
			}
		}

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
