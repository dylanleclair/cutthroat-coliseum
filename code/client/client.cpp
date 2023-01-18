#include <GL/glew.h>


#include <iostream>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"

#include "systems/ecs.h"

#include "CarPhysics.h"

#include "FrameCounter.h"

CarPhysics carPhysics;
CarPhysicsSerde carConfig(carPhysics);

std::vector<glm::vec3> drawFromMidpoints(std::vector<glm::vec3> src);
std::vector<glm::vec3> colourSquare(std::vector<glm::vec3> dest, glm::vec3 colour);

CPU_Geometry squaresDiamonds(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, int depth) {

	glm::vec3 squareColor(255 / 255, 255 / 255, 0 / 255);
	glm::vec3 diamondColor(255 / 255, 51 / 255, 153 / 255);

	CPU_Geometry cpu;

	cpu.verts.push_back(a);
	cpu.verts.push_back(b);
	cpu.verts.push_back(c);
	cpu.verts.push_back(d);
	
	cpu.cols = colourSquare(cpu.cols, squareColor);
	cpu.verts = drawFromMidpoints(cpu.verts);
	cpu.cols =colourSquare(cpu.cols, diamondColor);

	depth--;

	while (depth > 0)
	{

		for (int i = 0; i < 2; i++) {

			cpu.verts = drawFromMidpoints(cpu.verts);

			if (i == 0)
				cpu.cols = colourSquare(cpu.cols, squareColor);
			else cpu.cols = colourSquare(cpu.cols, diamondColor);


		}

		depth--;
	}

	return cpu;
}


std::vector<glm::vec3> drawFromMidpoints(std::vector<glm::vec3> src)
{

	std::vector<glm::vec3> output = src;

	int index = src.size() - 4;

	glm::vec3 a, b, c, d, e, f, g,h;

	a = output[index++];
	b = output[index++];
	c = output[index++];
	d = output[index];

	e = (0.5f * a + 0.5f * b);
	f = (0.5f * b + 0.5f * c);
	g = (0.5f * c + 0.5f * d);
	h = (0.5f * d + 0.5f * a);

	output.push_back(e);
	output.push_back(f);
	output.push_back(g);
	output.push_back(h);

	return output;
}

std::vector<glm::vec3> colourSquare(std::vector<glm::vec3> dest, glm::vec3 colour)
{
	std::vector<glm::vec3> colours = dest;

	for (int i = 0; i < 4; i++) {
		colours.push_back(colour);
	}

	return colours;


}


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
	Window window(800, 800, "CPSC 453");

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// GEOMETRY
	CPU_Geometry cpuGeom;
	GPU_Geometry gpuGeom;

	cpuGeom = squaresDiamonds(glm::vec3(0.8, -0.8, 0), glm::vec3(0.8, 0.8, 0), glm::vec3(-0.8, 0.8, 0), glm::vec3(-0.8, -0.8, 0), 3);


	for (int i = 0; i < cpuGeom.verts.size(); i++) {
		std::cout << cpuGeom.verts[i] << std::endl;
	}

	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);

	carConfig.deserialize();

	// init ecs ////////////////////////////////
	ecs::Scene mainScene;

	// spawn some entities.
	for (int i = 0; i < 10; i++)
	{
		ecs::Entity e = mainScene.CreateEntity();
		mainScene.AddComponent(e.guid, ExampleComponent{ 0,1,2 });
	}
	
	// create instance of system to use.
	ExampleSystem exampleEcsSystem;
	/////////////////////////////////////////////

	FramerateCounter framerate;
	// RENDER LOOP
	// while (!window.shouldClose()) {
	bool quit = false;
	while (!quit) {
		while (SDL_PollEvent(&window.event)) {
			ImGui_ImplSDL2_ProcessEvent(&window.event);

			if (window.event.type == SDL_QUIT)
				quit = true;

			if (window.event.type == SDL_KEYDOWN) {
				switch (window.event.key.keysym.sym) {
					case SDLK_r:
						shader.recompile();
						break;
					case SDLK_t:
						carConfig.deserialize();
						break;
					case SDLK_s:
						carConfig.serialize();
						break;
					default:
						break;
				};
			}
		}

		shader.use();
		gpuGeom.bind();


		// BEGIN ECS SYSTEMS UPDATES 
		exampleEcsSystem.Update(mainScene, 0.0f);
		// END__ ECS SYSTEMS UPDATES


		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (int i = 0; i < GLsizei(cpuGeom.verts.size()); i+=4)
		{
			glDrawArrays(GL_LINE_LOOP, i, 4);
		}

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Car Physics", nullptr);

		ImGui::SliderFloat("acceleration", &carPhysics.m_acceleration, 0.f, 1000.f);
		ImGui::SliderFloat("suspension", &carPhysics.m_suspension_force, 0.f, 1000.f);

		ImGui::Text(exampleEcsSystem.getDisplayString().c_str());

		ImGui::Text("framerate: %d", framerate.framerate());

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
