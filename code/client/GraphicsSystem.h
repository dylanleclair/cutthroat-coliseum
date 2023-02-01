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

#include "PxPhysicsAPI.h"

/*
* Render component is what is visible to the user
*/
struct RenderComponent
{
	GPU_Geometry* geom = new GPU_Geometry();
	int numVerts = 0;
	RenderComponent() = default;
	RenderComponent(CPU_Geometry* _geom) { geom->setCols(_geom->cols); geom->setVerts(_geom->verts); numVerts = _geom->verts.size(); }
};

/*
* Transform describes the objects position in 3D space.
*/
struct TransformComponent
{
	glm::vec3 position;
	glm::quat rotation;
};


struct GraphicsSystem : ecs::ISystem {
public:
	GraphicsSystem(Window& _window);
	void Update(ecs::Scene& scene, float deltaTime);
	void input(SDL_Event&, int _cameraID);
	static void readVertsFromFile(RenderComponent& _component, const std::string _file);
private:
	Camera cameras[4];
	int numCamerasActive = 1;
	GLint modelUniform = -1;
	GLuint viewUniform = -1;
	GLuint perspectiveUniform = -1;
	ShaderProgram shader;
	glm::ivec2 windowSize;
	static void processNode(aiNode* node, const aiScene* scene, CPU_Geometry* geom);
};