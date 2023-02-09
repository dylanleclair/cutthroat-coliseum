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
	static void readVertsFromFile(RenderComponent& _component, const std::string _file, const std::string _textureFile = "");
private:
	Camera cameras[4];
	int numCamerasActive = 1;
	GLint modelUniform = -1;
	GLuint viewUniform = -1;
	GLuint perspectiveUniform = -1;
	GLuint shaderSelectorUniform = -1;
	GLuint textureUniform = -1;
	ShaderProgram shader;
	glm::ivec2 windowSize;
	static void processNode(aiNode* node, const aiScene* scene, CPU_Geometry* geom);
};