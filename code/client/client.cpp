#define ImGUI_Enabled
#include <iostream>


#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include "systems/graphics/Geometry.h"
#include "PxPhysicsAPI.h"

#include "Window.h"

#include <unordered_map>

#include "core/ecs.h"
#include "systems/GraphicsSystem.h"
#include "systems/components.h"

#include "FrameCounter.h"

#include "ImGuiTuneables.h"

#include "utils/Time.h"
#include "Input.h"
#include "utils/PxConversionUtils.h"
#include "glm/gtx/string_cast.hpp"

#include "CenterMass.h"
#include "systems/PhysicsSystem.h"

#include "entities/car/Car.h"
#include "entities/car/CarUtils.h"

#include "entities/car/TireTracks.h"
#include "TetherGraphics.h"
#include "entities/physics/Obstacles.h"

#include "entities/physics/LevelCollider.h"

#include "systems/SoundSystem.h"

#include "systems/RaceSystem.h"

#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime



float startCountdown{5.0f};


bool loadLevelMesh{false};
bool levelMeshLoaded{false};

glm::vec3 calculateSpherePoint(float s, float t)
{
	float z = cos(2 * M_PI * t) * sin(M_PI * s);
	float x = sin(2 * M_PI * t) * sin(M_PI * s);
	float y = cos(M_PI * s);
	return(glm::vec3(x, y, z));
}

bool showImgui = true;

int lapCount = 0;
bool isFinished = false;

bool navPathToggle = true;

// Boolean to toggle gameplay mode
// (follow cam, full level mesh, navmesh off, backface culling off)
bool gameplayMode = false;
bool raceCountdown = false;
bool gamePaused = false;

uint32_t lastTime_millisecs;


void resetLevel(Car& testCar, std::vector<Guid> ais, ecs::Scene& mainScene, std::vector<glm::vec3> spawnPoints, RaceTracker& raceSystem, float& acc_t, glm::vec3 forward)
{
	glm::quat q = quatLookAt(forward, { 0,1.f,0 });

	// Ai Reset
	for (int i = 0; i < ais.size(); i++) {
		if (i == 0) {
			testCar.m_driverType = DriverType::HUMAN; // Hardcoding first player to be reset to a human, as there will always be one human player
		}
		if (i < ControllerInput::getNumberPlayers()) {
			testCar.m_driverType = DriverType::HUMAN;
		}
		Car& aiCar = mainScene.GetComponent<Car>(ais.at(i));
		aiCar.m_Vehicle.mPhysXState.physxActor.rigidBody->setGlobalPose(PxTransform(GLMtoPx(spawnPoints[i]),GLMtoPx(q)));
		aiCar.m_Vehicle.mPhysXState.physxActor.rigidBody->setLinearDamping(10000.f);
		aiCar.m_Vehicle.mPhysXState.physxActor.rigidBody->setAngularDamping(10000.f);
		aiCar.m_navPath->resetNav();
	}

	// Resets the lap count for all racers
	raceSystem.resetRace();

	// Resets the accumulator
	acc_t = 0;

	// Starting up the race countdown
	raceCountdown = true;
	startCountdown = 5.0f;

}

void gamePlayToggle(bool toggle, ecs::Scene &mainScene, std::vector<Guid> aiCars, GraphicsSystem &gs) {
	if (toggle) {
		loadLevelMesh = true;
		navPathToggle = false;
		gs.s_cameraMode(3); // follow cam

		raceCountdown = true;
		startCountdown = 5.0f;

		// Turns off the direction line for all AI
		for (int i = 0; i < aiCars.size(); i++) {
			RenderLine& AIDirection = mainScene.GetComponent<RenderLine>(aiCars.at(i));
			CPU_Geometry blank = CPU_Geometry();
			AIDirection.setGeometry(blank);
		}

		showImgui = true;
	}
	else {
		loadLevelMesh = false;
		navPathToggle = true;
		gs.s_cameraMode(1); // free cam

		raceCountdown = false;
		startCountdown = 0.f;

		// Restores the forward lines for the AI cars
		for (int i = 0; i < aiCars.size(); i++) {
			RenderLine& AIDirection = mainScene.GetComponent<RenderLine>(aiCars.at(i));
			CPU_Geometry forward = CPU_Geometry();
			forward.verts.push_back({ 0.f, 0.f, 0.f });
			forward.verts.push_back({ 0.f, 0.f, 5.f });
			AIDirection.setGeometry(forward);
		}
	}
}

int main(int argc, char* argv[]) {
	//RUN_GRAPHICS_TEST_BENCH();
	printf("Starting main\n");

	Window window(1200, 800, "Maximus Overdrive");

	lastTime_millisecs = SDL_GetTicks();

#ifdef ImGUI_Enabled
	ImGuiIO& io = ImGui::GetIO(); (void)io;
#endif

	/**
	 * Begin initialization of ECS systems, entities, etc.
	 * - Graphics
	 * - Physics
	 * - AI
	 */

	// first and foremost, create a scene.
	ecs::Scene mainScene;

	GraphicsSystem gs = GraphicsSystem();

	physics::PhysicsSystem physicsSystem{};
	physicsSystem.Initialize();



	CPU_Geometry zzPathGeom;
	GraphicsSystem::importSplineFromOBJ(zzPathGeom, "zz-track-nav.obj");
	
	glm::vec3 desiredSpawnLocation = {-4.108957, 3.397303, -43.794819}; // hardcoded value near the straight strip of the track
	Curve raceTrackingCurve(zzPathGeom.verts);

#ifdef ImGUI_Enabled
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
#endif
	

	// init ecs 

	// Static Friction, Dynamic Friction, Restitution
 	static float levelMaterial[3] = { 0.5f, 1.0f, 0.10f};

	std::cout << "Component initalization finished\n";




	// FIND SPAWNPOINTS FOR VEHICLES (player car is first)
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
	forward = -glm::normalize(forward);
	// need to pass the track spline to the car so it can compute the track normal...

	int spawnRows = 3;
	int spawnCols = 3;
	std::vector<glm::vec3> spawnPoints = spawnpointsAlongAxis(spawnRows,spawnCols, 10.f, forward, zzPathGeom.verts[zzSpawnIndex]);

	int numCars = spawnPoints.size();

	std::cout << "Starting race with " << numCars << " drivers!";

	/* --------------------------- */
	/* 	Spawn the cars						 */
	/* --------------------------- */

	// need to load the path for each ai
	// ai path is lower resolution curve to help prevent ai from "overcorrecting"
	// but this could also be fixed by fixing normal computation
	// in steering code
	CPU_Geometry nav_geom;
	GraphicsSystem::importSplineFromOBJ(nav_geom, "zz-track-ai-nav.obj");
	Curve aiNavigationPath{nav_geom.verts};


	std::vector<Guid> AIGuids;
	std::vector<NavPath> aiPaths;
	aiPaths.reserve(spawnPoints.size());

	ControllerInput::initControllers();

	// initialize the controllers
	// then count the number of human players

	// SPAWN THE AI CARS
	// skip the first spot (player driven vehicle) 
	for (int i = 0; i < spawnPoints.size(); i++)
	{		
		auto& spawnPoint = spawnPoints[i];
		aiPaths.emplace_back(&aiNavigationPath);
		auto& navPath = aiPaths[aiPaths.size() - 1];
		// Guid aiCarGuid = spawnAIEntity(mainScene, &physicsSystem, car_e.guid, spawnPoint, &navPath);
		Guid aiCarGuid = spawnCar(DriverType::COMPUTER, mainScene,&physicsSystem,spawnPoint, forward, &raceTrackingCurve, &navPath);

		AIGuids.push_back(aiCarGuid);
		setupCarVFX(mainScene, aiCarGuid);
	}


	// put them into human
	// conditionally spawn the other cars lmao ? 
	std::unordered_map<Guid, int> controllerMappings; // the controller -> car mapping

	int number_players = ControllerInput::getNumberPlayers();

	if (number_players == 0) number_players = 1; 

	gs.s_camerasActive(number_players);

	for (int i = 0; i < 4 && i < number_players; i++)
	{
		// get car at each guid
		// if it's under ze number_players
		// set it's driver to HUMAN
		Car& testCar = mainScene.GetComponent<Car>(AIGuids[i]);
		testCar.m_driverType = DriverType::HUMAN;
		testCar.controllerIndex = i;
		// controllerMappings[AIGuids[i]] = i;
		gs.bindCameraToEntity(i, AIGuids[i]);
	}

	// bandaids for other calls that do player-only stuff
	Car& testCar = mainScene.GetComponent<Car>(AIGuids[0]);
	Guid carGuid = AIGuids[0];

	ecs::Entity navRenderer_e = mainScene.CreateEntity();
	mainScene.AddComponent(navRenderer_e.guid,TransformComponent{});
	auto navPathRender = RenderLine{zzPathGeom};
	navPathRender.setColor(glm::vec3{1.0f,0.f,1.0f});
	mainScene.AddComponent(navRenderer_e.guid, navPathRender);

	// Transform component used for toggling the rendering
	auto& navRender = mainScene.GetComponent<TransformComponent>(navRenderer_e.guid);
	auto navDefaultScale = navRender.getScale();

	//add the flames
	ecs::Entity exhausePipes[4] = { mainScene.CreateEntity(), mainScene.CreateEntity(), mainScene.CreateEntity(), mainScene.CreateEntity() };
	for (int i = 0; i < 4; i++) {
		TransformComponent pipe_t = TransformComponent(testCar.getVehicleRigidBody());
		if (i == 0) 
			pipe_t.setPosition(glm::vec3(0.65, 1.54, 1.85));
		else if (i == 1)
			pipe_t.setPosition(glm::vec3(0.65, 1.54, 2.1));
		else if (i == 2)
			pipe_t.setPosition(glm::vec3(-0.65, 1.54, 1.85));
		else if (i == 3)
			pipe_t.setPosition(glm::vec3(-0.65, 1.54, 2.1));
		pipe_t.setScale(glm::vec3(1));
		VFXBillboard flame_b = VFXBillboard("textures/Fire.png", glm::vec3(1, 1, 0));
		/*
		VFXParticleSystem flame_p = VFXParticleSystem("textures/blackSmoke.png", 25);
		flame_p.initialVelocityMin = glm::vec3(-0.1, 0, -0.1);
		flame_p.initialVelocityMax = glm::vec3(0.1, 0.3, 0.1);
		flame_p.gravity = glm::vec3(0, 5, 0);
		flame_p.particleFrequency = 0.2;
		flame_p.initialScaleMin = 0.5;
		flame_p.initialScaleMax = 0.8;
		flame_p.particleLife = 0.3;
		flame_p.alphaChangeMin = -0.4;
		flame_p.alphaChangeMax = -0.6;
		flame_p.scaleSpeedMin = 0.3;
		flame_p.scaleSpeedMax = 0.5;
		*/
		mainScene.AddComponent(exhausePipes[i].guid, flame_b);
		mainScene.AddComponent(exhausePipes[i].guid, pipe_t);
		//mainScene.AddComponent(exhausePipes[i].guid, flame_p);
		
	}
	
	RaceTracker raceSystem{raceTrackingCurve, desiredSpawnLocation};	

	//make an entity
	ecs::Entity level_e = mainScene.CreateEntity();
	ecs::Entity outWall_e = mainScene.CreateEntity();
	ecs::Entity inWall_e = mainScene.CreateEntity();
	ecs::Entity ground_e = mainScene.CreateEntity();
	ecs::Entity finish_e = mainScene.CreateEntity();
	ecs::Entity tetherPole1_e = mainScene.CreateEntity();
	ecs::Entity tetherPole2_e = mainScene.CreateEntity();
	ecs::Entity tether_e = mainScene.CreateEntity();


	// Path renderer
	ecs::Entity path = mainScene.CreateEntity();
	mainScene.AddComponent(path.guid, TransformComponent{});
	mainScene.AddComponent(path.guid, RenderLine{});

	ecs::Entity new_level_e = mainScene.CreateEntity();

	TransformComponent new_level_t = TransformComponent();

	physx::PxMaterial* lMaterial = physicsSystem.m_Physics->createMaterial(0.10f, 0.730f, 0.135f);


	// LOAD COLLIDERS

	// load the road 
	CPU_Geometry new_level_geom = CPU_Geometry();
	GraphicsSystem::importOBJ(new_level_geom, "zz-track-collider-road.obj");

	Guid level_collider_e = mainScene.CreateEntity().guid;
	mainScene.AddComponent(level_collider_e, RoadCollider());
	RoadCollider& new_level_collider = mainScene.GetComponent<RoadCollider>(level_collider_e);
	new_level_collider.Initialize(new_level_geom, physicsSystem);
	physx::PxTriangleMesh* new_level_collider_mesh = new_level_collider.cookLevel(glm::scale(glm::mat4(1), glm::vec3(1.0)));
	new_level_collider.initLevelRigidBody(new_level_collider_mesh, lMaterial);

	// load the walls
	CPU_Geometry level_wall_geom = CPU_Geometry();
	GraphicsSystem::importOBJ(level_wall_geom, "zz-track-collider-wall.obj");

	Guid level_wall_e = mainScene.CreateEntity().guid;
	mainScene.AddComponent(level_wall_e, LevelCollider());
	LevelCollider& level_wall_collider = mainScene.GetComponent<LevelCollider>(level_wall_e);
	level_wall_collider.Initialize(level_wall_geom, physicsSystem);
	physx::PxTriangleMesh* level_wall_collider_mesh = level_wall_collider.cookLevel(glm::scale(glm::mat4(1), glm::vec3(1.0)));
	level_wall_collider.initLevelRigidBody(level_wall_collider_mesh, lMaterial);
	CPU_Geometry obstacle_geom = CPU_Geometry();
	GraphicsSystem::importOBJ(obstacle_geom, "obstacles-mesh.obj");

	gamePlayToggle(gameplayMode, mainScene, AIGuids, gs);


	// load the obstacles
	std::vector<Guid> obstacles;
	for (int i = 1; i <= 11; i++)
	{
		char buffer[50];

		sprintf(buffer, "obstacles/Pillar_%03d.obj", i);
		// load in obstacle collider & send it for rendering

		CPU_Geometry obstacle_geom = CPU_Geometry();
		GraphicsSystem::importOBJ(obstacle_geom, buffer);

		Guid obstacle_collider_e = mainScene.CreateEntity().guid;
		mainScene.AddComponent(obstacle_collider_e, ObstacleCollider());
		ObstacleCollider& new_obstacle_collider = mainScene.GetComponent<ObstacleCollider>(obstacle_collider_e);
		new_obstacle_collider.Initialize(obstacle_geom, physicsSystem);
		physx::PxTriangleMesh* new_obstacle_collider_mesh = new_obstacle_collider.cookLevel(glm::scale(glm::mat4(1), glm::vec3(1.0)));
		new_obstacle_collider.initLevelRigidBody(new_obstacle_collider_mesh, lMaterial);
		
	
		TransformComponent obs_t = TransformComponent();
		RenderModel obs_r = RenderModel();
		GraphicsSystem::importOBJ(obs_r,buffer);
		mainScene.AddComponent(obstacle_collider_e, obs_t);
		mainScene.AddComponent(obstacle_collider_e, obs_r);
	}

	ecs::Entity road_e = mainScene.CreateEntity();
	TransformComponent road_t = TransformComponent();
	RenderModel road_r = RenderModel();
	GraphicsSystem::importOBJ(road_r,"zz-track-road.obj");
	road_r.castsShadow = false;
	mainScene.AddComponent(road_e.guid, road_r);
	mainScene.AddComponent(road_e.guid, road_t);

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

	FramerateCounter framerate;

	bool quit = false;
	
	// Find the default values of movement dampening
	// This will be used to reset changes to dampening
	auto default_lin_damp = testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->getLinearDamping();
	auto default_ang_damp = testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->getAngularDamping();

	raceSystem.Initialize(mainScene);
	// Stuff for the physics timestep accumualtor
	// Previously was clamped

	auto previous_time = (float)SDL_GetTicks()/1000.f;

	float acc_t = 0.f;
	float delta_t = 1.f/60.f;

	// Sets up the better handling model on runtime 
	testCar.setup1();

	baseVariablesInit(testCar.m_Vehicle, physicsSystem);
	engineVariablesInit(testCar.m_Vehicle);

	float original_z_follow_dist = gs.follow_cam_z;

	bool playSounds = true;
	init_sound_system();
    SoundUpdater soundUpdater;
    soundUpdater.Initialize(mainScene);

	// button press frame data for the controller start buttons
	// used to prevent holding down behaviour triggering multiple
	// commands in a row
	bool start_button_previous_frame[4];
	for (int i = 0; i < 4; i++) {
		start_button_previous_frame[i] = false;
	}
	bool start_button_current_frame[4];
	for (int i = 0; i < 4; i++) {
		start_button_current_frame[i] = false;
	}

		std::cout << "initalization finished, beginning game\n";

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

		if (loadLevelMesh)
		{
			if (!levelMeshLoaded)
			{

				RenderModel new_level_r = RenderModel();
				GraphicsSystem::importOBJ(new_level_r,"zz-track-mesh.obj");
				//new_level_r.castsShadow = true;
				mainScene.AddComponent(new_level_e.guid, new_level_r);
				mainScene.AddComponent(new_level_e.guid, new_level_t);

				levelMeshLoaded = true;
			}
		}

		// Reset dampening values if they are changed every frame (used after car is reset) 
		//TODO:  May need to put in an if check, and factor out ?
		testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->setLinearDamping(default_lin_damp);
		testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->setAngularDamping(default_ang_damp);

		for (int i = 0; i < AIGuids.size(); i++) {
			Car& aiCar = mainScene.GetComponent<Car>(AIGuids.at(i));
			aiCar.m_Vehicle.mPhysXState.physxActor.rigidBody->setLinearDamping(default_lin_damp);
			aiCar.m_Vehicle.mPhysXState.physxActor.rigidBody->setAngularDamping(default_ang_damp);
		}

		// Update the Imgui every frame (Might cause performance issues) 
		baseVariablesInit(testCar.m_Vehicle, physicsSystem);
		engineVariablesInit(testCar.m_Vehicle);

		//polls all pending input events until there are none left in the queue
		SDL_Event windowEvent;
		while (SDL_PollEvent(&windowEvent)) {
#ifdef ImGUI_Enabled
			ImGui_ImplSDL2_ProcessEvent(&windowEvent);
#endif

			if (windowEvent.type == SDL_CONTROLLERDEVICEADDED) {
				std::cout << "Adding controller\n";
				ControllerInput::init_controller();
			}

			if (windowEvent.type == SDL_CONTROLLERDEVICEREMOVED) {
				std::cout << "removing controller\n";
				ControllerInput::deinit_controller();
			}

			if (windowEvent.type == SDL_QUIT)
				quit = true;

			if (windowEvent.type == SDL_KEYDOWN) {
				switch (windowEvent.key.keysym.sym) {

					case SDLK_r:
						//TODO recompile the shader
						
						resetLevel(testCar, AIGuids,mainScene,spawnPoints, raceSystem, acc_t, forward);
						break;
						
					// TODO: change the file that is serializes (Want to do base.json and enginedrive.json)
					case SDLK_i:// i means in
						break;
					case SDLK_o:// o means out
						break;
					case SDLK_F1:
						if (gamePaused) {
							gamePaused = false;
						}
						else {
							gamePaused = true;
						}
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
							testCar.m_Vehicle.mPhysXState.physxActor.rigidBody->setGlobalPose(PxTransform(GLMtoPx(spawnPoints[0])));
							

						break;
					case SDLK_SPACE:
						testCar.Jump();
						break;
				case SDLK_c:
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
			gs.input(windowEvent, 0);
		}

		// Iterates through all player controllers and
		// checks if the start or select button has been pressed
		// and launches the approriate functions 
		for (int i = 0; i < number_players; i++)
		{
			Car& testCar = mainScene.GetComponent<Car>(AIGuids[i]);

			// Checks the controller input and saves the value for the current frame
			start_button_current_frame[i] = testCar.carGetControllerStartPressed();

			// Checks if the current frame had no button press, and the current frame had a button press
			// This is to prevent "holding down button" behaviour triggering the game to be paused on and off
			// repeatiedly 
			if (start_button_current_frame[i] && !start_button_previous_frame[i]) {

				if (gamePaused) { gamePaused = false; }
				else if (!gamePaused) { gamePaused = true; }
			}
			
			if (testCar.carGetControllerSelectPressed()) {
				resetLevel(testCar, AIGuids, mainScene, spawnPoints, raceSystem, acc_t, forward);				
			}

			// Saves the current frame data as the previous frame for the next frame go around
			start_button_previous_frame[i] = start_button_current_frame[i];
		}


		// Check for the car grounded state, and if grounded after being in the air
		// resets the modifications made to the car while in the air
		// This is because angular dampening is applied while in the air to prevent
		// the car from flipping, but on the ground it can't turn so it resets
		// the angular dampening on the ground
		if (testCar.isGroundedDelay(testCar)) {
			testCar.resetModifications();
		}	

		// Used to toggle the nav path rendering 
		if (navPathToggle) {
			navRender.setScale(navDefaultScale);
		}
		else {
			navRender.setScale(vec3(0));
		}

		// Timestep accumulate for proper physics stepping
		auto current_time = (float)SDL_GetTicks() / 1000.f;
		auto time_diff = current_time - previous_time;
		if (time_diff > 0.25f) {
			time_diff = 0.25f;
		}
		previous_time = current_time;

		// If the game isn't paused - update physics 
		if (!gamePaused) {
			acc_t = acc_t + (time_diff);
			while (acc_t >= delta_t) {
				acc_t = acc_t - delta_t;
				physicsSystem.Update(mainScene, delta_t);
			}
		}


		//flame effects for ONLY the player car
		{
			static float acc = 0;
			acc += time_diff;
			if (acc > 0.01) {
				acc = 0;
				for (int i = 0; i < 4; i++) {
					const float scaleWindow = 0.15; //how much it can vary to make the changes less abrupt
					const float minScale = 0.3;
					const float maxScale = 1.7;
					const float velocity = glm::length(PxtoGLM(testCar.getVehicleRigidBody()->getLinearVelocity()));

					const float windowPosition = (velocity / (velocity + 15)) + minScale;
					const float scale = glm::linearRand(max(windowPosition - scaleWindow, 0.f), min(maxScale, windowPosition + scaleWindow));
					TransformComponent& trans = mainScene.GetComponent<TransformComponent>(exhausePipes[i].guid);
					glm::vec3 position = trans.getLocalTranslation();
					position.y = 0.44 * scale + 1.1;
					trans.setPosition(position);
					trans.setScale(glm::vec3(scale));
					//VFXParticleSystem& part = mainScene.GetComponent<VFXParticleSystem>(exhausePipes[i].guid);
					//part.stepSystem(delta_t, trans.getTransformationMatrix());
				}
			}
		}
		
		// Tire track renders
		updateCarVFX(mainScene, time_diff);
		gs.Update(mainScene, time_diff);
		raceSystem.Update(mainScene,time_diff);

		//update_sounds(testCar, aiCarInstance, playSounds);
        soundUpdater.Update(mainScene, time_diff);

		// END__ ECS SYSTEMS UPDATES

#ifdef ImGUI_Enabled
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		if (showImgui) {
			ImGui::Begin("Milestone 4 debug panel");
			if (ImGui::BeginTabBar("Debug Tab Bar")) {
				if (ImGui::BeginTabItem("Performance")) {
					// BEGIN FRAMERATE COUNTER
					framerate.update(time_diff * 1000.f);
					ImGui::SetNextWindowSize(ImVec2(500, 100));
					ImGui::Text("framerate: %d", (int)framerate.framerate());
					ImGui::PlotLines("Frametime plot (ms)", framerate.m_time_queue_ms.data(), framerate.m_time_queue_ms.size());
					ImGui::PlotLines("Framerate plot (hz)", framerate.m_rate_queue.data(), framerate.m_rate_queue.size());
					ImGui::SliderFloat3("Level material params", levelMaterial, 0.0f, 5.0f);

					// END FRAMERATE COUNTER

					if (!loadLevelMesh)
					{
						ImGui::Checkbox("Load level mesh", &loadLevelMesh);
					}
					
					// Used to toggle a bunch of stuff at the same time for gameplay
					if (ImGui::Checkbox("Gameplay Mode", &gameplayMode)) {
						gamePlayToggle(gameplayMode, mainScene, AIGuids, gs);
					}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Vehicle")) {
					// Car PhysX variable panels
					testCar.carImGui();
					//ImGui Panels for tuning
					//reloadVehicleJSON();
					if(ImGui::CollapsingHeader("Vehicle Tuning"))
						vehicleTuning(testCar.m_Vehicle, physicsSystem);
					if (ImGui::CollapsingHeader("Engine Tuning"))
						engineTuning(testCar.m_Vehicle);
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Graphics")) {
					// Graphics imgui panel for graphics tuneables
					gs.ImGuiPanel();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Misc")) {
					// Obstacles ImGui
					obstaclesImGui(mainScene, physicsSystem);
					ImGui::Checkbox("Nav Path Render", &navPathToggle);
					tireTrackImgui();
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::End();

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
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Lap: %d/%d", raceSystem.getLapCount(carGuid), raceSystem.MAX_LAPS);
		ImGui::PopFont();
		ImGui::End();
		
		//Lap counter
		ImGui::SetNextWindowPos(ImVec2(10, 30));
		ImGui::Begin("UI", (bool*)0, textWindowFlags);
		ImGui::SetWindowFontScale(2.f);
		ImGui::PushFont(CabalBold);
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Rank: %d/%d", raceSystem.getRanking(carGuid), spawnPoints.size() );
		ImGui::PopFont();
		ImGui::End();


		//you win message
		static int counter = 0;
		const float delayInSeconds = 0.5;
		static bool display = true;
		if (raceSystem.getRaceStatus()) {
			// the race is finished!!
			
			// make the AI take over after the driver has finished (UI will take precedence)
			testCar.m_driverType = DriverType::COMPUTER;

			// see if the message should be displayed
			counter += timestep.getMilliseconds();
			if (counter >= delayInSeconds * 1000) {
				counter = 0;
				display = !display;
			}

			// check who the winner was
			const char * winner = (raceSystem.getRanking(carGuid) == 1) ? "VICTORY!" : "AI WON!";

			if (display) {
				ImGui::SetNextWindowPos(ImVec2(200, 200));
				ImGui::Begin("UI2", (bool*)0, textWindowFlags);
				ImGui::SetWindowFontScale(5.f);
				ImGui::PushFont(CabalBold);
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), winner);
				ImGui::PopFont();
				ImGui::End();
			}

		}

		if (raceCountdown)
		{
			if (startCountdown > 0.f)
			{
				gamePaused = true;
				startCountdown -= timestep.getSeconds();
			
				ImGui::SetNextWindowPos(ImVec2(200, 200));
				ImGui::Begin("UI2", (bool*)0, textWindowFlags);
				ImGui::SetWindowFontScale(2.5f);
				ImGui::PushFont(CabalBold);
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Race starting in... %.0f", startCountdown + 1);
				ImGui::PopFont();
				ImGui::End();
			
			}else  { 
				raceCountdown = false;
				gamePaused = false; 
			}
		} 

#endif
		//glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		window.RenderAndSwap();
	}
#ifdef ImGUI_Enabled
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
#endif

	// cleanupPhysics();
	physicsSystem.Cleanup();

	ControllerInput::deinit_controller();

	SDL_Quit();
	return 0;
}
