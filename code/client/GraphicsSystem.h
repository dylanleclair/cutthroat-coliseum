#pragma once
#include "systems/ecs.h"
#include <GL/glew.h>
#include "Geometry.h"
#include "Window.h"
#include "Camera.h"
#include "Position.h"
#include "ShaderProgram.h"
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct RenderComponent
{
	CPU_Geometry* geom;
	Position* position;
	RenderComponent() {} //this doesn't do anything, but it needs to be here
	RenderComponent(CPU_Geometry* _geom, Position* _position) : geom(_geom), position(_position) {  }
};

struct MeshComponent {
	MeshComponent() {}
	MeshComponent(std::string _file);
	void processNode(aiNode* node, const aiScene* scene, CPU_Geometry* geom);
	CPU_Geometry* geom = new CPU_Geometry;
private:
	bool initalized = false;
	
};

struct GraphicsSystem : ecs::ISystem {
public:
	GraphicsSystem(Window& _window);
	void Update(ecs::Scene& scene, float deltaTime);
	void input(SDL_Event&, int _cameraID);
private:
	Camera cameras[4];
	int numCamerasActive = 1;
	GLint modelUniform = -1;
	GLuint viewUniform = -1;
	GLuint perspectiveUniform = -1;
	ShaderProgram shader;
	glm::ivec2 windowSize;
};

