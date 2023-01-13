#include "GraphicsSystem.h"
#include <GL/glew.h>
#include "Geometry.h"
#include <iostream>
#include "GLFW/glfw3.h"

GraphicsSystem::GraphicsSystem()

void GraphicsSystem::addPrimitive(render_packet _packet)
{
	glPointSize(10);
	// GEOMETRY
	CPU_Geometry cpuGeom;
	GPU_Geometry gpuGeom;

	cpuGeom.verts.push_back(glm::vec3(0));
	cpuGeom.cols.push_back(glm::vec3(1));

	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);

	gpuGeom.bind();

	std::cout << "SHHHHHHHHHHHHHHH\n";

	glDrawArrays(GL_POINTS, 0, 1);
}

void GraphicsSystem::render() {

}
