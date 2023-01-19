#include "GraphicsSystem.h"
#include <GL/glew.h>
#include <iostream>
#include "GLDebug.h"
#include "ShaderProgram.h"
#include "glm/gtc/type_ptr.hpp"
#include "Camera.h"
#include "Position.h"

GraphicsSystem::GraphicsSystem(Window& _window) : shader("shaders/test.vert", "shaders/test.frag") {
	// SHADERS
	shader.use();

	//get uniform locations
	modelUniform = glGetUniformLocation(GLuint(shader), "M");
	viewUniform = glGetUniformLocation(GLuint(shader), "V");
	perspectiveUniform = glGetUniformLocation(GLuint(shader), "P");
}

void GraphicsSystem::addPrimitive(render_packet _packet)
{
	//NOTE: I believe this should copy the CPU_Geometry into the vector. This isn't ideal since whenever the vector resizes it will need to copy all the data but the ECS should take care of this
	geometries.push_back(_packet);
}

void GraphicsSystem::render() {
	glPointSize(10);
	// GEOMETRY
	GPU_Geometry gpuGeom;

	gpuGeom.bind();
	gpuGeom.setVerts(geometries[0].geom.verts);
	gpuGeom.setCols(geometries[0].geom.cols);

	//matricies
	glm::mat4 P = glm::perspective(glm::radians(45.0f), 1.0f, 0.01f, 1000.f);
	glm::mat4 V = camera.getView();
	glm::mat4 M = glm::mat4(1.0f);//geometries[0].position.getTransformMatrix();

	glUniformMatrix4fv(perspectiveUniform, 1, GL_FALSE, glm::value_ptr(P));
	glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(V));
	glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(M));

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.use();

	glPointSize(10.0f);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void GraphicsSystem::input(SDL_Event& _event)
{
	camera.input(_event);
}
