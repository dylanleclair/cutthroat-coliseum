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
	~GraphicsSystem();
	void ImGuiPanel();
	void renderUI();
	void Update(ecs::Scene& scene, float deltaTime);
	void input(SDL_Event&, int _cameraID);
	glm::mat4 getCameraView();
	glm::vec3 GraphicsSystem::g_cameraPosition();
	glm::vec3 GraphicsSystem::g_cameraVelocity();
	static void importOBJ(CPU_Geometry& _geometry, const std::string _fileName);
	static void importOBJ(RenderModel& _component, const std::string _fileName);
private:

	Camera cameras[4];
	//uniforms
	int numCamerasActive = 1;

	int cam_mode = 1; // Used to determine what mode the camera should use (free, fixed, follow)
	glm::mat4 V = glm::mat4(1.f); // Had to declare this variable here for the rest of the program to work

	//debug panel variables
	bool showColliders = false;
	struct renderableInfo {
		std::vector<Guid> ids; 
		std::vector<std::string> names; 
		std::vector<glm::vec3> positions; 
		std::vector <glm::vec3> scales;
		std::vector <glm::vec4> rotations;
		std::vector <char> read_write; //0 = nothing, 1 = read, 2 = write
		int count = 0;
	};
	renderableInfo entityTransforms;

	float depthDiffWeight = 1;
	float normalDiffWeight = 1;
	glm::vec3 lightDirection = glm::vec3(1, -3, 1);
	float ambiantStrength = 1;
	float diffuseWeight = 0.3;
	int numQuantizedSplits = 10; //CAN'T BE 1!!!

	//shader variables
	ShaderProgram modelShader;
	ShaderProgram lineShader;
	ShaderProgram wireframeShader;
	ShaderProgram gShader;
	ShaderProgram celShader;
	//buffer for offscreen rendering
	GLuint gBuffer;
	GLuint gDepth;
	GLuint gColor;
	GLuint gNormal;
	GLuint gPosition;
	//variables for rendering whole screen quad
	GLuint quad_vertexArray;
	GLuint quad_vertexBuffer;

	glm::ivec2 windowSize;
	static void processNode(aiNode* node, const aiScene* scene, RenderModel& _component);
	static void processNode(aiNode* node, const aiScene* scene, CPU_Geometry& _geometry);

	//follo camera variables
	float follow_cam_x;
	float follow_cam_y;
	float follow_cam_z;
	float follow_correction_strength;
	bool faceCulling;
	bool front_face;
	bool back_face;
};