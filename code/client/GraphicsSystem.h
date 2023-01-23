#pragma once
#include "systems/ecs.h"
#include <GL/glew.h>
#include "Geometry.h"
#include "Window.h"
#include "Camera.h"
#include "Position.h"
#include "ShaderProgram.h"

//a package of data telling the system what primitive to draw
struct render_packet {
	CPU_Geometry geom;
	Position position;
	render_packet(CPU_Geometry& _geom, Position _position) : geom(_geom), position(_position) {}
};


struct GraphicsSystem : ecs::ISystem {
public:
	GraphicsSystem(Window& _window);
	void addPrimitive(render_packet _packet);
	void Update(ecs::Scene& scene, float deltaTime);
	void input(SDL_Event&, int _cameraID);
private:
	std::vector<render_packet> geometries;
	Camera cameras[4];
	int numCamerasActive = 1;
	GLint modelUniform = -1;
	GLuint viewUniform = -1;
	GLuint perspectiveUniform = -1;
	ShaderProgram shader;
	glm::ivec2 windowSize;
};