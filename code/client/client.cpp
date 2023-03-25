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
#include "utils/PxConversionUtils.h"

#include "systems/PhysicsSystem.h"

#include "entities/car/Car.h"
#include "entities/car/AICar.h"
#include "entities/car/AIEntity.h"


#include "TetherGraphics.h"
#include "entities/physics/Obstacles.h"

#include "entities/physics/LevelCollider.h"

#include "systems/SoundSystem.h"

#include "systems/RaceSystem.h"

#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime

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

std::vector<glm::vec3> spawnpointsAlongAxis(int rows, int cols,float spread, glm::vec3 axis, glm::vec3 start)
{
	std::vector<glm::vec3> result;

	axis = glm::normalize(axis);
	const glm::vec3 UP = {0.f, 1.f,0.f};
	// spawn the cars along the axis
	glm::vec3 binormal = glm::cross(axis,UP);
	// spawn a row of cars on binormal of axis

	for (int col =0; col < cols; col++)
	{
		glm::vec3 colStart = start + (-axis * (col * spread));
		for (int row = 0; row < rows; row++)
		{
			glm::vec3 spawnPosition = colStart + (binormal * (row * spread));
			result.push_back(spawnPosition);
		}
	}


	return result;

}

int main(int argc, char* argv[]) {
	//RUN_GRAPHICS_TEST_BENCH();
	printf("Starting main");


	const int driverCount = 8;



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

	CPU_Geometry zzPathGeom;
	GraphicsSystem::importSplineFromOBJ(zzPathGeom, "zz-track-nav.obj");

	RaceTracker raceSystem{zzPathGeom.verts, glm::vec3{-4.108957, 3.397303, -43.794819}};	


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

 	static float levelMaterial[3] = { 0.10f, 0.730f, 0.135f};

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

	ecs::Entity tether_e = mainScene.CreateEntity();

	// ecs::Entity aiDriver_e = mainScene.CreateEntity();

	mainScene.AddComponent(car_e.guid, Car{});
	Car& testCar = mainScene.GetComponent<Car>(car_e.guid);
	testCar.physicsSystem = &physicsSystem;
	
	if (!testCar.initVehicle(PxVec3(-4.108957, 3.397303, -43.794819)))
	{
		std::cout << "ERROR: could not initialize vehicle";
	}

	NavPath circlePath = generateCirclePath(30);

	CPU_Geometry aiPathGeom;
	// import the obj for the path
	GraphicsSystem::importOBJ(aiPathGeom, "ai_path.obj");
	// transform verts same way level will be???

	// PATHFINDING FOR NEW TRACK

	std::cout << "zz track navmesh has " << zzPathGeom.verts.size() << " vertices" << std::endl;

	glm::vec3 desiredSpawnLocation = {-4.108957, 3.397303, -43.794819};
	int zzSpawnIndex = 0;
	float minDistToSpawn = std::numeric_limits<float>::max();
	for (int i = 0; i < zzPathGeom.verts.size(); i++)
	{	
		float currDistance = glm::distance(zzPathGeom.verts[i], desiredSpawnLocation);
		if (currDistance < minDistToSpawn)
		{
			zzSpawnIndex = i;
			minDistToSpawn = currDistance;
		}
	}

	glm::vec3 forward = (zzSpawnIndex == zzPathGeom.verts.size() - 1) ? zzPathGeom.verts[0] - zzPathGeom.verts[zzSpawnIndex] : zzPathGeom.verts[zzSpawnIndex + 1] - zzPathGeom.verts[zzSpawnIndex];

	// generate spawnpoints along the axis!
	std::vector<glm::vec3> aiSpawnPoints = spawnpointsAlongAxis(2,3, 5.f, forward, zzPathGeom.verts[zzSpawnIndex]);

	// find the point on the track to desired spawn location

	auto scaling =  glm::scale(glm::mat4{1.f},glm::vec3(3.2f, 3.2f, 3.2f));
	for (auto& vert : aiPathGeom.verts)
	{
			vert = scaling * glm::vec4{vert,1.f}; 
			vert.y = 0.f;
	}

	std::vector<NavPath> aiPaths;
	aiPaths.reserve(aiSpawnPoints.size());

	for (auto& spawnPoint : aiSpawnPoints)
	{
		aiPaths.emplace_back(zzPathGeom.verts);
		auto& navPath = aiPaths[aiPaths.size() - 1];
		Guid aiCarGuid = spawnAIEntity(mainScene, &physicsSystem, car_e.guid, spawnPoint, &navPath);
		AICar& aiCarInstance = mainScene.GetComponent<AICar>(aiCarGuid);
		// idk why we get the car instance tbh
	}

	// NavPath aiPath{zzPathGeom.verts};


	// NavPath aiPath{aiPathGeom.verts};

	ecs::Entity navRenderer_e = mainScene.CreateEntity();
	mainScene.AddComponent(navRenderer_e.guid,TransformComponent{});
	auto navPathRender = RenderLine{zzPathGeom};
	navPathRender.setColor(glm::vec3{1.0f,0.f,1.0f});
	mainScene.AddComponent(navRenderer_e.guid,navPathRender);




	// Car Entity
	RenderModel car_r = RenderModel();
	GraphicsSystem::importOBJ(car_r, "alpha_cart.obj");
	car_r.setModelColor(glm::vec3(0.5f, 0.5f, 0.f));
	car_r.isShadowed(true);
	mainScene.AddComponent(car_e.guid, car_r);
	TransformComponent car_t = TransformComponent(testCar.getVehicleRigidBody());
	car_t.setPosition(glm::vec3(0, -0.3f, 0.5f));
	car_t.setScale(glm::vec3(3.2f, 3.2f, 3.2f));
	mainScene.AddComponent(car_e.guid, car_t);
	


	
	// Finish line components
	// RenderModel finish = RenderModel();
	// GraphicsSystem::importOBJ(finish, "basic_finish.obj");
	// finish.setModelColor(glm::vec3(1.f, 0.f, 0.f));
	// mainScene.AddComponent(finish_e.guid, finish);

	// TransformComponent finish_t = TransformComponent();
	// finish_t.setPosition(glm::vec3(30, 0, 0));
	// finish_t.setScale(glm::vec3(3.2f, 3.2f, 3.2f));
	// mainScene.AddComponent(finish_e.guid, finish_t);

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
	// CPU_Geometry levelColliderFloor_raw = CPU_Geometry();
	// GraphicsSystem::importOBJ(levelColliderFloor_raw, "STADIUM_COLLIDER_FLOOR.obj");
	// LevelCollider levelCollider1{ levelColliderFloor_raw, physicsSystem};
	// auto levelTriangleMesh1 = levelCollider1.cookLevel(glm::scale(glm::mat4(1), glm::vec3(3.2)));
	// levelCollider1.initLevelRigidBody(levelTriangleMesh1);

	// CPU_Geometry levelColliderInner_raw = CPU_Geometry();
	// GraphicsSystem::importOBJ(levelColliderInner_raw, "STADIUM_COLLIDER_INNER.obj");
	// LevelCollider levelCollider2{ levelColliderInner_raw, physicsSystem };
	// auto levelTriangleMesh2 = levelCollider2.cookLevel(glm::scale(glm::mat4(1), glm::vec3(3.2)));
	// levelCollider2.initLevelRigidBody(levelTriangleMesh2);

	// CPU_Geometry levelColliderOuter_raw = CPU_Geometry();
	// GraphicsSystem::importOBJ(levelColliderOuter_raw, "STADIUM_COLLIDER_OUTER.obj");
	// LevelCollider levelCollider3{ levelColliderOuter_raw, physicsSystem };
	// auto levelTriangleMesh3 = levelCollider3.cookLevel(glm::scale(glm::mat4(1), glm::vec3(3.2)));
	// levelCollider3.initLevelRigidBody(levelTriangleMesh3);

	//Level
	// RenderModel level_r = RenderModel();
	// //GraphicsSystem::importOBJ(level_r, "Stadium.obj");
	// GraphicsSystem::importOBJ(level_r, "Stadium_MINIMAL.obj"); //for faster loading times
	// mainScene.AddComponent(level_e.guid, level_r);
	// mainScene.AddComponent(level_e.guid, level_t);

	ecs::Entity new_level_e = mainScene.CreateEntity();

	TransformComponent new_level_t = TransformComponent();
	level_t.setScale(glm::vec3(1.f, 1.f, 1.f));

	physx::PxMaterial* lMaterial = physicsSystem.m_Physics->createMaterial(0.10f, 0.730f, 0.135f);

	CPU_Geometry new_level_geom = CPU_Geometry();
	GraphicsSystem::importOBJ(new_level_geom, "zz-track.obj");
	LevelCollider new_level_collider{ new_level_geom, physicsSystem };
	auto new_level_collider_mesh = new_level_collider.cookLevel(glm::scale(glm::mat4(1), glm::vec3(1.0)));
	new_level_collider.initLevelRigidBody(new_level_collider_mesh, lMaterial);


	RenderModel new_level_r = RenderModel();
	GraphicsSystem::importOBJ(new_level_r,"zz-track.obj");
	mainScene.AddComponent(new_level_e.guid, new_level_r);
	mainScene.AddComponent(new_level_e.guid, new_level_t);


	// // Tether poles
	// RenderModel tetherPole1_r = RenderModel();
	// GraphicsSystem::importOBJ(tetherPole1_r, "alpha_tether_pole.obj");
	// tetherPole1_r.setModelColor(glm::vec3(205.f / 255.f, 133.f / 255.f, 63.f / 255.f));
	// mainScene.AddComponent(tetherPole1_e.guid, tetherPole1_r);
	// TransformComponent tetherPole1_t = TransformComponent();
	// tetherPole1_t.setPosition(glm::vec3(0.f, 0.f, 162.f));
	// tetherPole1_t.setScale(glm::vec3(3.2f, 3.2f, 3.2f));
	// mainScene.AddComponent(tetherPole1_e.guid, tetherPole1_t);
	
	// RenderModel tetherPole2_r = RenderModel();
	// GraphicsSystem::importOBJ(tetherPole2_r, "alpha_tether_pole.obj");
	// tetherPole2_r.setModelColor(glm::vec3(205.f / 255.f, 133.f / 255.f, 63.f / 255.f));
	// mainScene.AddComponent(tetherPole2_e.guid, tetherPole2_r);
	// TransformComponent tetherPole2_t = TransformComponent();
	// tetherPole2_t.setPosition(glm::vec3(0.f, 0.f, -162.f));
	// tetherPole2_t.setScale(glm::vec3(3.2f, 3.2f, 3.2f));
	// mainScene.AddComponent(tetherPole2_e.guid, tetherPole2_t);
	
	// // Tether
	// RenderModel tether_r = RenderModel();
	// GraphicsSystem::importOBJ(tether_r, "alpha_tether.obj");
	// tether_r.setModelColor(glm::vec3(83.f / 255.f, 54.f / 255.f, 33.f / 255.f));
	// mainScene.AddComponent(tether_e.guid, tether_r);
	// TransformComponent tether_t = TransformComponent();
	// tether_t.setPosition(glm::vec3(0.f, 1.f, 0.f));
	// tether_t.setScale(glm::vec3(1.f, 2.f, 2.f));
	// mainScene.AddComponent(tether_e.guid, tether_t);

	//tire tracks for the main player cart
	//front tire
	ecs::Entity frontTireTrack = mainScene.CreateEntity();
	VFXTextureStrip frontTireTrack_r = VFXTextureStrip("textures/MotercycleTireTread.png", 0.07, 2);
	frontTireTrack_r.maxLength = 25;
	TransformComponent frontTireTrack_t = TransformComponent();
	mainScene.AddComponent(frontTireTrack.guid, frontTireTrack_r);
	mainScene.AddComponent(frontTireTrack.guid, frontTireTrack_t);
	//right tire
	ecs::Entity rightTireTrack = mainScene.CreateEntity();
	VFXTextureStrip rightTireTrack_r = VFXTextureStrip("textures/MotercycleTireTread.png", 0.07, 1);
	rightTireTrack_r.maxLength = 15;
	TransformComponent rightTireTrack_t = TransformComponent();
	mainScene.AddComponent(rightTireTrack.guid, rightTireTrack_r);
	mainScene.AddComponent(rightTireTrack.guid, rightTireTrack_t);
	//left tire
	ecs::Entity leftTireTrack = mainScene.CreateEntity();
	VFXTextureStrip leftTireTrack_r = VFXTextureStrip("textures/MotercycleTireTread.png", 0.07, 1);
	leftTireTrack_r.maxLength = 15;
	TransformComponent leftTireTrack_t = TransformComponent();
	mainScene.AddComponent(leftTireTrack.guid, leftTireTrack_r);
	mainScene.AddComponent(leftTireTrack.guid, leftTireTrack_t);

	/*
	* Demonstration of the Billboard Component. It always expects a texture to be used and an optinal locking axis can be used
	* The Billboard will always try to face the camera
	*/
	ecs::Entity billboard = mainScene.CreateEntity();
	VFXBillboard bill_r = VFXBillboard("textures/CFHX3384.JPG", glm::vec3(0, 1, 0));
	TransformComponent bill_t = TransformComponent();
	bill_t.setPosition(glm::vec3(0, 20, 0));
	bill_t.setScale(glm::vec3(10, 5, 0));
	mainScene.AddComponent(billboard.guid, bill_r);
	mainScene.AddComponent(billboard.guid, bill_t);


	// Setting up log obstacles (currently boxes)
	// setUpLogs(mainScene);
	// addRigidBody(physicsSystem);

	// This is how to change the position of the object after it has been passed to the ECS
	/*
	auto &wallTrans = mainScene.GetComponent<TransformComponent>(outWall_e.guid);
	wallTrans.setPosition(glm::vec3(0, 0, 0));
	*/
	
	// Fetching ecs components for system call debugs and other
	// debug functionality
	// auto &finish_trans = mainScene.GetComponent<TransformComponent>(finish_e.guid);
	// TransformComponent &car_trans = mainScene.GetComponent<TransformComponent>(car_e.guid);
	// TransformComponent &sphere_transform = mainScene.GetComponent<TransformComponent>(sphere_e.guid);
	// TransformComponent &tetherPole1_transform = mainScene.GetComponent<TransformComponent>(tetherPole1_e.guid);
	// TransformComponent &tetherPole2_transform = mainScene.GetComponent<TransformComponent>(tetherPole2_e.guid);
	// TransformComponent &tether_transform = mainScene.GetComponent<TransformComponent>(tether_e.guid);
	// PxTransform loc;

	// std::vector<TransformComponent> c_tether_points;
	// c_tether_points.push_back(tetherPole1_transform);
	// c_tether_points.push_back(tetherPole2_transform);

	FramerateCounter framerate;

	bool quit = false;
	int controlledCamera = 0;
	


	// Find the default values of movement dampening
	// This will be used to reset changes to dampening
	auto default_lin_damp = testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->getLinearDamping();
	auto default_ang_damp = testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->getAngularDamping();

	raceSystem.Initialize(mainScene);
	// Stuff for the physics timestep accumualtor
	// Previously was clamped

	auto previous_time = (float)SDL_GetTicks()/1000.f;

	float acc_t = 0.f;
	const float delta_t = 1.f/60.f;

	// Sets up the better handling model on runtime 
	testCar.setup1();

	baseVariablesInit(testCar.m_Vehicle, physicsSystem);
	engineVariablesInit(testCar.m_Vehicle);

	bool playSounds = true;
	init_sound_system();

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

		// Update the Imgui every frame (Might cause performance issues) 
		baseVariablesInit(testCar.m_Vehicle, physicsSystem);
		engineVariablesInit(testCar.m_Vehicle);

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
						testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->setGlobalPose(PxTransform(PxVec3(-4.108957, 3.397303, -43.794819)));
						testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->setLinearDamping(10000.f);
						testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->setAngularDamping(10000.f);
						lapCount = 1;
						// TODO: apply the dampening to ai when resetting the ai
						// Will need to for loop all ai cars
						//aiCarInstance.m_Vehicle.mPhysXState.physxActor.rigidBody->setGlobalPose(PxTransform(10.f, 2.f, 10.f));

						// Resets the accumulator
						acc_t = 0;

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
					case SDLK_9:
						 new_level_collider.release();
						 new_level_collider.initLevelRigidBody(new_level_collider_mesh, physicsSystem.m_Physics->createMaterial(levelMaterial[0], levelMaterial[1], levelMaterial[2]));
							testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->setGlobalPose(PxTransform(PxVec3(-4.108957, 3.397303, -43.794819)));

						break;
					case SDLK_SPACE:
						testCar.TetherJump();
						break;
					case SDLK_m:
						if (!testCar.getCTethered()) {
							// loc.p.x = tetherPole1_transform.getTranslation().x;
							// loc.p.y = tetherPole1_transform.getTranslation().y;
							// loc.p.z = tetherPole1_transform.getTranslation().z;
							// testCar.TetherSteer(loc);
						}
						else if (testCar.getCTethered()) {
							// testCar.resetModifications();
						}

						break;


					// Prinout of camera matrix
				case SDLK_c:
					// std::cout << gs.getCameraView()[0][0] << ", " << gs.getCameraView()[0][1] << ", " << gs.getCameraView()[0][2] << ", " << gs.getCameraView()[0][3] << "," << std::endl;
					// std::cout << gs.getCameraView()[1][0] << ", " << gs.getCameraView()[1][1] << ", " << gs.getCameraView()[1][2] << ", " << gs.getCameraView()[1][3] << "," << std::endl;
					// std::cout << gs.getCameraView()[2][0] << ", " << gs.getCameraView()[2][1] << ", " << gs.getCameraView()[2][2] << ", " << gs.getCameraView()[2][3] << "," << std::endl;
					// std::cout << gs.getCameraView()[3][0] << ", " << gs.getCameraView()[3][1] << ", " << gs.getCameraView()[3][2] << ", " << gs.getCameraView()[3][3] << std::endl;
					// std::cout << std::endl;

					// std::cout << "finish line: " << finish_trans.getTranslation().x << ", " << finish_trans.getTranslation().y << ", " << finish_trans.getTranslation().z << std::endl;
					// std::cout << std::endl;

					// std::cout << "tether pole 1: " << tetherPole1_t.getTranslation().x << ", " << tetherPole1_t.getTranslation().y << "," << tetherPole1_t.getTranslation().z << std::endl;
					// std::cout << std::endl;

					// std::cout << "Car Transform: " << std::endl;

					// std::cout << car_trans.getTranslation().x << ", " << car_trans.getTranslation().y << ", " << car_trans.getTranslation().z << std::endl;


					// std::cout << mainScene.GetComponent<TransformComponent>(car_e.guid).getTranslation().x << ","
					// 	<< mainScene.GetComponent<TransformComponent>(car_e.guid).getTranslation().y << ","
					// 	<< mainScene.GetComponent<TransformComponent>(car_e.guid).getTranslation().z << std::endl;

					// std::cout << testCar.getVehicleRigidBody()->getGlobalPose().p.x << ", "
					// 		  << testCar.getVehicleRigidBody()->getGlobalPose().p.y << ", "
					// 	      << testCar.getVehicleRigidBody()->getGlobalPose().p.z << std::endl;
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

		
		// // Finish line code
		// if (car_trans.getTranslation().x >= 28.f && car_trans.getTranslation().x <= 40.f &&
		// 	car_trans.getTranslation().z >= -2.f && car_trans.getTranslation().z <= 0.f)
		// {
		// 	if (isFinished == false) {
		// 		isFinished = true;
		// 		finishLinePrint();
		// 	}

		// }
		// else {
		// 	isFinished = false;
		// }

		// Stuff to check engine rotation speed and steering response
		// Used for debugging and tuning the vehicle 
		// CAN DELETE LATER
		float percent_rot = testCar.m_Vehicle.mEngineDriveState.engineState.rotationSpeed / testCar.m_Vehicle.mEngineDriveParams.engineParams.maxOmega;
		percent_rot = 1.f - percent_rot;
		//testCar.m_Vehicle.mBaseParams.steerResponseParams.maxResponse = percent_rot * 1.52;

		//tire track logic
		static bool previousState[3] = { false, false, false };
		VFXTextureStrip& frontTireTracks = mainScene.GetComponent<VFXTextureStrip>(frontTireTrack.guid);
		VFXTextureStrip& rightTireTracks = mainScene.GetComponent<VFXTextureStrip>(rightTireTrack.guid);
		VFXTextureStrip& leftTireTracks = mainScene.GetComponent<VFXTextureStrip>(leftTireTrack.guid);
		//front tire
		if (testCar.m_Vehicle.mBaseState.roadGeomStates[0].hitState && testCar.m_Vehicle.mBaseState.roadGeomStates[1].hitState) {
			previousState[0] = true;
			glm::vec3 frontTirePosition = PxtoGLM(testCar.getVehicleRigidBody()->getGlobalPose().p) + glm::vec3(PxtoGLM(testCar.getVehicleRigidBody()->getGlobalPose().q) * glm::vec4(0, 0, 4, 1));
			if (glm::length(frontTirePosition - frontTireTracks.g_previousPosition()) > 1) {
				frontTireTracks.extrude(frontTirePosition, glm::vec3(0, 1, 0));
			}
			else {
				//frontTireTracks.moveEndPoint(frontTirePosition, glm::vec3(0, 1, 0));
			}
		}
		else {
			if (previousState[0] == true)
				frontTireTracks.cut();
			previousState[0] = false;
		}
		
		//right tire
		if (testCar.m_Vehicle.mBaseState.roadGeomStates[2].hitState) {
			previousState[1] = true;
			glm::vec3 rightTirePosition = PxtoGLM(testCar.getVehicleRigidBody()->getGlobalPose().p) + glm::vec3(PxtoGLM(testCar.getVehicleRigidBody()->getGlobalPose().q) * glm::vec4(1, 0, 0, 1));
			if (glm::length(rightTirePosition - rightTireTracks.g_previousPosition()) > 1) {
				rightTireTracks.extrude(rightTirePosition, glm::vec3(0, 1, 0));
			}
			else {
				//rightTireTracks.moveEndPoint(rightTirePosition, glm::vec3(0, 1, 0));
			}
		}
		else {
			if (previousState[1] == true)
				rightTireTracks.cut();
			previousState[1] = false;
		}

		//left tire
		if (testCar.m_Vehicle.mBaseState.roadGeomStates[3].hitState) {
			previousState[2] = true;
			glm::vec3 leftTirePosition = PxtoGLM(testCar.getVehicleRigidBody()->getGlobalPose().p) + glm::vec3(PxtoGLM(testCar.getVehicleRigidBody()->getGlobalPose().q) * glm::vec4(-1, 0, 0, 1));
			if (glm::length(leftTirePosition - leftTireTracks.g_previousPosition()) > 1) {
				leftTireTracks.extrude(leftTirePosition, glm::vec3(0, 1, 0));
			}
			else {
				//leftTireTracks.moveEndPoint(leftTirePosition, glm::vec3(0, 1, 0));
			}
		}
		else {
			if (previousState[2] == true)
				leftTireTracks.cut();
			previousState[2] = false;
		}

		// Timestep accumulate for proper physics stepping
		auto current_time = (float)SDL_GetTicks()/1000.f;
		auto time_diff = current_time - previous_time;
		if (time_diff > 0.25f) {
			time_diff = 0.25f;
		}
		previous_time = current_time;

		acc_t = acc_t + (time_diff);
		while (acc_t >= delta_t) {
			acc_t = acc_t - delta_t;
			physicsSystem.Update(mainScene, delta_t);
		}

		gs.Update(mainScene, time_diff);

		//update_sounds(testCar, aiCarInstance, playSounds);

		// END__ ECS SYSTEMS UPDATES

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		if (showImgui) {
			// BEGIN FRAMERATE COUNTER
			framerate.update(time_diff * 1000.f);
			ImGui::SetNextWindowSize(ImVec2(500, 100));
			ImGui::Begin("Milestone 4");
			ImGui::Text("framerate: %d", (int)framerate.framerate());
			ImGui::PlotLines("Frametime plot (ms)", framerate.m_time_queue_ms.data(), framerate.m_time_queue_ms.size());
			ImGui::PlotLines("Framerate plot (hz)", framerate.m_rate_queue.data(), framerate.m_rate_queue.size());
			ImGui::SliderFloat3("Level material params", levelMaterial, 0.0f, 5.0f);
			ImGui::End();
			// END FRAMERATE COUNTER

			// Car PhysX variable panels
			testCar.carImGui();

			// NOTE: the imgui bible - beau
			//ImGui::ShowDemoWindow();

			// Graphics imgui panel for graphics tuneables
			gs.ImGuiPanel();

			//ImGui Panels for tuning
			//reloadVehicleJSON();
			vehicleTuning(testCar.m_Vehicle, physicsSystem);
			engineTuning(testCar.m_Vehicle);

			// Obstacles ImGui
			obstaclesImGui(mainScene, physicsSystem);
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
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Lap: %d/3", raceSystem.getLapCount(car_e.guid));
		ImGui::PopFont();
		ImGui::End();

		//Lap counter
		ImGui::SetNextWindowPos(ImVec2(10, 20));
		ImGui::Begin("UI", (bool*)0, textWindowFlags);
		ImGui::SetWindowFontScale(2.f);
		ImGui::PushFont(CabalBold);
		//ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "AI Lap: %d/3", aiCarInstance.m_lapCount);
		ImGui::PopFont();
		ImGui::End();

		
		//Lap counter
		ImGui::SetNextWindowPos(ImVec2(10, 30));
		ImGui::Begin("UI", (bool*)0, textWindowFlags);
		ImGui::SetWindowFontScale(2.f);
		ImGui::PushFont(CabalBold);
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Rank: %d/%d", raceSystem.getRanking(car_e.guid), 1 + aiSpawnPoints.size() );
		ImGui::PopFont();
		ImGui::End();


		//you win message
		static int counter = 0;
		const float delayInSeconds = 0.5;
		static bool display = true;
		if (raceSystem.getLapCount(car_e.guid) >= 3) {
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

		// else if (aiCarInstance.m_lapCount >= 3) {
		// 	counter += timestep.getMilliseconds();
		// 	if (counter >= delayInSeconds * 1000) {
		// 		counter = 0;
		// 		display = !display;
		// 	}
		// 	if (display) {
		// 		ImGui::SetNextWindowPos(ImVec2(200, 200));
		// 		ImGui::Begin("UI2", (bool*)0, textWindowFlags);
		// 		ImGui::SetWindowFontScale(5.f);
		// 		ImGui::PushFont(CabalBold);
		// 		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "AI VICTORY");
		// 		ImGui::PopFont();
		// 		ImGui::End();
		// 	}
		// }

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
