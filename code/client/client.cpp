#include <iostream>


#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"


#include "graphics/Geometry.h"

#include "Window.h"

#include "systems/ecs.h"
//#include "systems/PhysicsSystem.h"
#include "systems/GraphicsSystem.h"
#include "systems/components.h"

#include "CarPhysics.h"
#include "FrameCounter.h"


using namespace physx;

extern bool initPhysics();
extern void stepPhysics();
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
	init_physics();

	// init ecs 
	ecs::Scene mainScene;

	//make a ground plane
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
	physx::PxRigidStatic* groundPlane = physx::PxCreatePlane(*gPhysics, physx::PxPlane(0, 1, 0, 50), *gMaterial);
	gScene->addActor(*groundPlane);

	//make a cube entity
	ecs::Entity e = mainScene.CreateEntity();

	//create and place a cube
	float halfLen = 0.5f;
	physx::PxTransform tran(physx::PxVec3(0, 50, -30)); //put the cube 40 units in the air
	physx::PxRigidDynamic* body = gPhysics->createRigidDynamic(tran);

	//for the physx visual debugger.
	physx::PxShape* shape = gPhysics->createShape(physx::PxBoxGeometry(halfLen, halfLen, halfLen), *gMaterial);
	body->attachShape(*shape);

	physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
	gScene->addActor(*body);

	RenderComponent rend = RenderComponent();
	GraphicsSystem::readVertsFromFile(rend, "models/torus.obj");
	mainScene.AddComponent(e.guid, rend);

	TransformComponent trans = TransformComponent(body);
	mainScene.AddComponent(e.guid, trans);


	
	
	std::cout << "Component initalization finished\n";

	// create instance of system to use.
	GraphicsSystem gs(window);

	//init_physx();
	
	if (initPhysics())
	{
		std::cout << "initialized physx driving model\n";
	}
	

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
		//std::cout << "Beginning system updates\n";
		if(framerate.m_time_queue.size() != 0)
			gScene->simulate(framerate.m_time_queue.front() / 1000.0f);
		else
			gScene->simulate(0.1);
		gScene->fetchResults(true); //block until the simulation is finished

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
			//float frame_time_seconds = framerate.m_time_queue.front() / 1000.0f;
			//gScene->simulate(frame_time_seconds);
			//gScene->fetchResults(true);
		}

		// PHYSX DRIVER UPDATE
		stepPhysics();


		// TODO(milestone 1): strip all non-milestone related imgui windows out
		// BEGIN CAR PHYSICS PANEL
		ImGui::Begin("Car Physics", nullptr);
		ImGui::SliderFloat("acceleration", &carPhysics.m_acceleration, 0.f, 1000.f);
		ImGui::SliderFloat("suspension", &carPhysics.m_suspension_force, 0.f, 1000.f);
		if (ImGui::Button("Serialize")) carConfig.serialize();
		ImGui::End();
		// END CAR PHYSICS PANEL

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

	SDL_Quit();
	return 0;
}
