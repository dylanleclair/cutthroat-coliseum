#include "GraphicsSystem.h"
#include <GL/glew.h>
#include "Geometry.h"
#include <iostream>
#include "GLFW/glfw3.h"
#include "GraphicsSystem.h"
#include "GLDebug.h"
#include "ShaderProgram.h"

GraphicsSystem::GraphicsSystem(Window& _window) {
	// WINDOW
	glfwInit();

	GLDebug::enable();

	// SHADERS
	shader = ShaderProgram("shaders/test.vert", "shaders/test.frag");

	GLint uniMat = glGetUniformLocation(shader, "M");
}

void GraphicsSystem::addPrimitive(render_packet _packet)
{

}

void GraphicsSystem::render() {
	glPointSize(10);
	// GEOMETRY
	CPU_Geometry cpuGeom;
	GPU_Geometry gpuGeom;

	cpuGeom.verts.push_back(glm::vec3(0));
	cpuGeom.cols.push_back(glm::vec3(1));

	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);

	gpuGeom.bind();

	glDrawArrays(GL_POINTS, 0, 1);
}
