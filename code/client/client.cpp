#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"

#include "GraphicsSystem.h"


// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(ShaderProgram& shader) : shader(shader) {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			shader.recompile();
		}
	}

private:
	ShaderProgram& shader;
};


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


int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired

	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// CALLBACKS
	window.setCallbacks(std::make_shared<MyCallbacks>(shader)); // can also update callbacks to new ones

	// GEOMETRY
	CPU_Geometry cpuGeom;
	GPU_Geometry gpuGeom;

	cpuGeom = squaresDiamonds(glm::vec3(0.8, -0.8, 0), glm::vec3(0.8, 0.8, 0), glm::vec3(-0.8, 0.8, 0), glm::vec3(-0.8, -0.8, 0), 3);


	for (int i = 0; i < cpuGeom.verts.size(); i++) {
		std::cout << cpuGeom.verts[i] << std::endl;
	}

	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);

	GraphicsSystem gs();

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		shader.use();
		gpuGeom.bind();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//std::cout << GLsizei(cpuGeom.verts.size());

		for (int i = 0; i < GLsizei(cpuGeom.verts.size()); i+=4)
		{
			std::cout << " " << i << std::endl;

			glDrawArrays(GL_LINE_LOOP, i, 4);
			
		}

		render_packet pack(sphere, 4, 0, 0);
		gs.addPrimitive(pack);

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
