#pragma once
#include "systems/ecs.h"
#include <GL/glew.h>
#include "graphics/Geometry.h"
#include "../client/Window.h"
#include "graphics/Camera.h"
#include "graphics/ShaderProgram.h"
#include "graphics/Texture.h"
#include "components.h"
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "PxPhysicsAPI.h"

#include <glm/gtc/quaternion.hpp> 
#include <glm/gtx/quaternion.hpp>


struct GraphicsSystem : ecs::ISystem {
public:
	GraphicsSystem(Window& _window);
	void Update(ecs::Scene& scene, float deltaTime);
	void input(SDL_Event&, int _cameraID);
	glm::mat4 getCameraView();
	static void importOBJ(RenderModel& _component, const std::string _fileName);
private:
	Camera cameras[4];
	//uniforms
	int numCamerasActive = 1;

	ShaderProgram modelShader;
	ShaderProgram lineShader;
	glm::ivec2 windowSize;
	static void processNode(aiNode* node, const aiScene* scene, RenderModel& _component);
};