#include "core/ecs.h"
#include "GraphicsSystem.h"

#include <GL/glew.h>
#include <iostream>
#include "graphics/GLDebug.h"
#include "graphics/ShaderProgram.h"
#include "glm/gtc/type_ptr.hpp"
#include "graphics/Camera.h"
#include "glm/gtc/quaternion.hpp"
#include <glm/gtx/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <PxPhysicsAPI.h>

#include <stdio.h>
#include "stb_image.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <ctype.h>


#include "graphics/Texture.h"

//DEBUG IMPORTS
#include "graphics/snippetrender/SnippetRender.h"



void loadCubemap(std::vector<std::string> faces)
{
	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

GraphicsSystem::GraphicsSystem(Window& _window) :
	modelShader("shaders/lighting_simple.vert", "shaders/lighting_simple.frag"),
	lineShader("shaders/line.vert", "shaders/line.frag"),
	wireframeShader("shaders/wireframe.vert", "shaders/wireframe.frag"),
	gShader("shaders/gShader.vert", "shaders/gShader.frag"),
	celShader("shaders/cel.vert", "shaders/cel.frag"),
	shadowGShader("shaders/shadowMap.vert", "shaders/shadowMap.frag"),
	VFXshader("shaders/VFX.vert", "shaders/VFX.frag"),
	skyboxShader("shaders/skybox.vert", "shaders/skybox.frag"),
	particleShader("shaders/particle.vert", "shaders/particle.frag")
{
	windowSize = _window.getSize();
	follow_cam_x = 0.f;
	follow_cam_y = 4.f;
	follow_cam_z = -22.f;
	follow_correction_strength = 40.f;
	maximum_follow_distance = 10;
	front_face = false;
	back_face = true;

	/*
	* create all textures
	*/
	// gposition texture
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowSize.x, windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// gnormal texture
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowSize.x, windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	// gcolor texture
	glGenTextures(1, &gColor);
	glBindTexture(GL_TEXTURE_2D, gColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowSize.x, windowSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	// gshadow texture
	glGenTextures(1, &gShadow);
	glBindTexture(GL_TEXTURE_2D, gShadow);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, windowSize.x, windowSize.y, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	// gdepth texture
	glGenTextures(1, &gDepth);
	glBindTexture(GL_TEXTURE_2D, gDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowSize.x, windowSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// gVFXcolor texture
	glGenTextures(1, &gVFXColor);
	glBindTexture(GL_TEXTURE_2D, gVFXColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowSize.x, windowSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//gVFXdepth texture
	glGenTextures(1, &gVFXDepth);
	glBindTexture(GL_TEXTURE_2D, gVFXDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowSize.x, windowSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	 
	// light depth texture
	glGenTextures(1, &gLightDepth);
	glBindTexture(GL_TEXTURE_2D, gLightDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_HALF_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1, 1, 1, 1 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	//skybox cubemap texture
	glGenTextures(1, &skyboxCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap);
	glActiveTexture(GL_TEXTURE0);
	std::vector<std::string> faces{"textures/-X.jpg","textures/+X.jpg","textures/+Y.jpg","textures/-Y.jpg","textures/-Z.jpg","textures/+Z.jpg"};
	loadCubemap(faces);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	/*
	* create and configure g-buffer framebuffer
	*/
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	//configure the g-buffers outputs
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColor, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gShadow, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepth, 0);
	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "generative framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/*
	* create and configure the VFX framebuffer
	*/
	glGenFramebuffers(1, &gVFXBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gVFXBuffer);
	//configure buffer outputs
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gVFXColor, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gVFXDepth, 0);
	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments2[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments2);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "generative framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	/*
	* create and configure the light depth framebuffer
	*/
	glGenFramebuffers(1, &gShadowBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gShadowBuffer);
	//configure light framebuffer outputs
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gLightDepth, 0);
	//tell openGL that we are generating no color data
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "light framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	/*
	* configure the skybox shader input textures
	*/
	skyboxShader.use();
	glUniform1i(glGetUniformLocation(GLuint(skyboxShader), "skybox"), 0);

	/*
	* configure the cel shader input textures
	*/
	celShader.use();
	glUniform1i(glGetUniformLocation(GLuint(celShader), "gPosition"), 0);
	glUniform1i(glGetUniformLocation(GLuint(celShader), "gNormal"), 1);
	glUniform1i(glGetUniformLocation(GLuint(celShader), "gColor"), 2);
	glUniform1i(glGetUniformLocation(GLuint(celShader), "gDepth"), 3);
	glUniform1i(glGetUniformLocation(GLuint(celShader), "gShadow"), 4);
	glUniform1i(glGetUniformLocation(GLuint(celShader), "gVFXColor"), 5);
	glUniform1i(glGetUniformLocation(GLuint(celShader), "gVFXDepth"), 6);
	glUniform1i(glGetUniformLocation(GLuint(celShader), "cubemap"), 7);
	
	gShader.use();
	glUniform1i(glGetUniformLocation(GLuint(gShader), "gShadowDepth"), 0);
	glUniform1i(glGetUniformLocation(GLuint(gShader), "diffTexture"), 1);

	//generate the data for the full screen render quad
	const GLfloat g_quad_vertex_buffer_data[] = {
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	1.0f,  1.0f, 0.0f,
	};
	glGenBuffers(1, &quad_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
	
	glGenVertexArrays(1, &quad_vertexArray);
	glBindVertexArray(quad_vertexArray);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//generate the data for the billboard effect
	const GLfloat billboard_vertex_buffer_data[] = {
		//x, y, z
	-.5f, -.5f, 0, 0, 0,
	.5f, -.5f, 0, 1, 0,
	-.5f,  .5f, 0, 0, 1,
	-.5f,  .5f, 0, 0, 1,
	.5f, -.5f, 0, 1, 0,
	.5f,  .5f, 0, 1, 1
	};
	glGenBuffers(1, &billboard_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(billboard_vertex_buffer_data), billboard_vertex_buffer_data, GL_STATIC_DRAW);

	glGenVertexArrays(1, &billboard_vertexArray);
	glBindVertexArray(billboard_vertexArray);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float)*3));
	glEnableVertexAttribArray(1);

	//generate the data for the skybox
	const float skyboxVertices[] = {
		// positions          
		-5.0f,  5.0f, -5.0f,
		-5.0f, -5.0f, -5.0f,
		 5.0f, -5.0f, -5.0f,
		 5.0f, -5.0f, -5.0f,
		 5.0f,  5.0f, -5.0f,
		-5.0f,  5.0f, -5.0f,

		-5.0f, -5.0f,  5.0f,
		-5.0f, -5.0f, -5.0f,
		-5.0f,  5.0f, -5.0f,
		-5.0f,  5.0f, -5.0f,
		-5.0f,  5.0f,  5.0f,
		-5.0f, -5.0f,  5.0f,

		 5.0f, -5.0f, -5.0f,
		 5.0f, -5.0f,  5.0f,
		 5.0f,  5.0f,  5.0f,
		 5.0f,  5.0f,  5.0f,
		 5.0f,  5.0f, -5.0f,
		 5.0f, -5.0f, -5.0f,

		-5.0f, -5.0f,  5.0f,
		-5.0f,  5.0f,  5.0f,
		 5.0f,  5.0f,  5.0f,
		 5.0f,  5.0f,  5.0f,
		 5.0f, -5.0f,  5.0f,
		-5.0f, -5.0f,  5.0f,

		-5.0f,  5.0f, -5.0f,
		 5.0f,  5.0f, -5.0f,
		 5.0f,  5.0f,  5.0f,
		 5.0f,  5.0f,  5.0f,
		-5.0f,  5.0f,  5.0f,
		-5.0f,  5.0f, -5.0f,

		-5.0f, -5.0f, -5.0f,
		-5.0f, -5.0f,  5.0f,
		 5.0f, -5.0f, -5.0f,
		 5.0f, -5.0f, -5.0f,
		-5.0f, -5.0f,  5.0f,
		 5.0f, -5.0f,  5.0f
	};
	glGenBuffers(1, &skybox_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, skybox_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &skybox_vertexArray);
	glBindVertexArray(skybox_vertexArray);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//instanced particle data
	const GLfloat particle_vertex_buffer_data[] = {
		//x, y
	-.5f, -.5f,
	.5f, -.5f,
	-.5f,  .5f,
	-.5f,  .5f,
	.5f, -.5f,
	.5f,  .5f
	};
	glGenVertexArrays(1, &particles_vertexArray);
	glBindVertexArray(particles_vertexArray);
	glGenBuffers(1, &particles_quadVertexBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, particles_quadVertexBuffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &particles_instanceTransformBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_instanceTransformBuffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(0));
	glVertexAttribDivisor(1, 1);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(sizeof(float) * 4));
	glVertexAttribDivisor(2, 1);

	glBindBuffer(GL_ARRAY_BUFFER, particles_quadVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particle_vertex_buffer_data), particle_vertex_buffer_data, GL_STATIC_DRAW);
}

GraphicsSystem::~GraphicsSystem() {
	glDeleteFramebuffers(1, &gBuffer);
	glDeleteTextures(1, &gColor);
	glDeleteTextures(1, &gNormal);
	glDeleteTextures(1, &gDepth);
	glDeleteTextures(1, &gPosition);
	glDeleteVertexArrays(1, &quad_vertexArray);
	glDeleteBuffers(1, &quad_vertexBuffer);
}

// Panel to controls the cameras
void GraphicsSystem::ImGuiPanel() {
	if (ImGui::CollapsingHeader("Camera")) {
		ImGui::Checkbox("Culling: Front Face", &front_face);
		ImGui::Checkbox("Culling: Back Face", &back_face);

		if (ImGui::Button("Free Camera")) {
			cam_mode = 1;
		}
		if (ImGui::Button("Fixed Camera")) {
			cam_mode = 2;
		}
		if (ImGui::Button("Follow Camera")) {
			cam_mode = 3;
		}

		if (cam_mode == 3) {
			ImGui::InputFloat("X Distance: ", &follow_cam_x);
			ImGui::InputFloat("Y Distance: ", &follow_cam_y);
			ImGui::InputFloat("Z Distance: ", &follow_cam_z);
			ImGui::InputFloat("Correction Strength", &follow_correction_strength);
			ImGui::InputFloat("max follow distance", &maximum_follow_distance);
		}
	}
	if (ImGui::CollapsingHeader("Rendering")) {
		ImGui::SliderFloat3("Light Direction", &(lightDirection.x), -50, 50);
		ImGui::SliderFloat("diffuse strength", &diffuseWeight, 0, 1);
		ImGui::SliderFloat("ambiant strength", &ambiantStrength, 0, 1);
		ImGui::SliderFloat("normalWeightDiff", &normalDiffWeight, 0, 1);
		ImGui::SliderFloat("depthWeightDiff", &depthDiffWeight, 0, 1);
		ImGui::SliderInt("number of color zones", &numQuantizedSplits, 0, 40);
		ImGui::SliderFloat("gooch strength", &goochStrength, 0, 1);
		ImGui::ColorPicker3("gooch cool", &(goochCool[0]));
		ImGui::ColorPicker3("gooch warm", &(goochWarm[0]));
	}
	if (ImGui::CollapsingHeader("Transforms")) {
		ImGui::Checkbox("Collider Meshes", &showColliders);
		//show all renderables in a list
		static int item_current_idx = 0;
		if (entityTransforms.count > 0) { //make sure there is at least one entity. Issues might otherwise arise
			if (ImGui::BeginCombo("Entities", entityTransforms.names[item_current_idx].c_str())) {
				for (int i = 0; i < entityTransforms.count; i++) {
					const bool is_selected = (item_current_idx == i);
					if (ImGui::Selectable(entityTransforms.names[i].c_str(), is_selected))
						item_current_idx = i;
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}


		static glm::vec3 pos = glm::vec3(0);
		static glm::vec4 rot = glm::vec4(0); //x, y, z, angle
		static glm::vec3 sca = glm::vec3(0);
		ImGui::InputFloat3("position", &(pos.x));
		ImGui::InputFloat4("rotation", &(rot.x));
		ImGui::InputFloat3("scale", &(sca.x));

		static bool reading = false;
		if (reading) {
			reading = false;
			pos = entityTransforms.positions[item_current_idx];
			rot = glm::vec4(glm::vec3(entityTransforms.rotations[item_current_idx]), glm::degrees(entityTransforms.rotations[item_current_idx][3]));
			sca = entityTransforms.scales[item_current_idx];
		}
		if (ImGui::Button("Read")) {
			reading = true;
			entityTransforms.read_write[item_current_idx] = 1;
		}

		if (ImGui::Button("Write")) {
			//save to the vector
			entityTransforms.positions[item_current_idx] = glm::vec3(pos);
			entityTransforms.rotations[item_current_idx] = glm::vec4(glm::vec3(rot), glm::radians(rot[3]));
			entityTransforms.scales[item_current_idx] = glm::vec3(sca);
			entityTransforms.read_write[item_current_idx] = 2;
		}
	}
}

glm::mat4 getViewMatrix(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up) {
	// Calculate the right vector
	glm::vec3 right = glm::normalize(glm::cross(up, direction));
	// Recalculate the up vector
	glm::vec3 newUp = glm::cross(direction, right);

	// Create a 4x4 view matrix
	glm::mat4 view = glm::mat4(1.0f);
	view[0][0] = right.x;
	view[1][0] = right.y;
	view[2][0] = right.z;
	view[0][1] = newUp.x;
	view[1][1] = newUp.y;
	view[2][1] = newUp.z;
	view[0][2] = -direction.x;
	view[1][2] = -direction.y;
	view[2][2] = -direction.z;
	view[3][0] = -glm::dot(right, position);
	view[3][1] = -glm::dot(newUp, position);
	view[3][2] = glm::dot(direction, position);

	return view;
}

void GraphicsSystem::Update(ecs::Scene& scene, float deltaTime) {
	for (int i = 0; i < numCamerasActive; i++) {
		//default camera matricies
		glm::mat4 P = glm::perspective(glm::radians(45.f), (float)windowSize.x / windowSize.y, 2.f, 1000.f);
		glm::mat4 V = cameras[i].getView();

		// If camera mode is 1 - use freecam
		if (cam_mode == 1) {
			V = cameras[i].getView();
		}
		// If cam mode is 2 - use fixed camera (values from milestone 2)
		else if (cam_mode == 2) {
			V = { 0.658686, -0.565264, 0.496598, 0,
				0, 0.660003, 0.751263, 0,
				-0.752418, -0.494847, 0.434735, 0,
				9.27202, -0.914308, -33.4781, 1
			};
		}
		// TODO: Follow camera mode
		else {
			//MASSIVE ASSUMPTION!!! Car is ALWAYS Guid = 0
			//TODO: Find a clean way to pass the cars Guid to the camera. Idealy also refactor the camera class to handle this logic
			//Main goal rn: Implement the logic...
			static glm::vec3 previousCarPosition = glm::vec3(0);
			TransformComponent& trans = scene.GetComponent<TransformComponent>(0);
			/*
			* calculate the camera position
			*/
			//calculate where the camera should aim to be positioned
			glm::vec3 cameraTargetLocation = glm::translate(glm::mat4(1), trans.getTranslation()) * toMat4(trans.getRotation()) * glm::vec4(follow_cam_x, follow_cam_y, follow_cam_z, 1);
			//calculate the speed of the car
			float speed = glm::distance(previousCarPosition, trans.getTranslation());
			//calculate the vector from the cameras current position to the target position
			glm::vec3 cameraOffset = cameras[0].getPos() - cameraTargetLocation;
			if (glm::length(cameraOffset) != 0) {
				float followDistance = (speed / (speed + follow_correction_strength)) * maximum_follow_distance;
				cameraOffset = glm::normalize(cameraOffset) * followDistance;
				cameras[0].setPos(cameraTargetLocation + cameraOffset);
			}
			//set the camera variables
			previousCarPosition = trans.getTranslation();
			V = glm::lookAt(cameras[0].getPos(), trans.getTranslation(), glm::vec3(0.0f, 1.0f, 0.0f));
			P = glm::perspective(glm::radians(45.f), (float)windowSize.x / windowSize.y, 2.f, 1000.f);
		}



		//set the viewport
		//if (numCamerasActive <= 1) { //there can't be 0 cameras, assume always 1 minimum
			glViewport(0, 0, windowSize.x, windowSize.y);
		//}
		/*else {
			if (i == 0) {
				glViewport(0, windowSize.y / 2, windowSize.x / 2, windowSize.y / 2);
			}
			else if (i == 1) {
				glViewport(windowSize.x / 2, windowSize.y / 2, windowSize.x / 2, windowSize.y / 2);
			}
			else if (i == 2) {
				glViewport(windowSize.x / 2, 0, windowSize.x / 2, windowSize.y / 2);
			}
			else if (i == 3) {
				glViewport(0, 0, windowSize.x / 2, windowSize.y / 2);
			}
		}*/

		/*
		* UPDATE THE TRANSFORM COMPONENTS
		*/
		for (Guid entityGuid : ecs::EntitiesInScene<RenderModel, TransformComponent>(scene)) {
			RenderModel& comp = scene.GetComponent<RenderModel>(entityGuid);
			TransformComponent& trans = scene.GetComponent<TransformComponent>(entityGuid);
			bool cont = false;
			//search if it is already in the debug list, and if it is update it
			for (int i = 0; i < entityTransforms.count; i++) {
				if (entityTransforms.ids[i] == entityGuid) {
					//read
					if (entityTransforms.read_write[i] == 1) {
						entityTransforms.positions[i] = trans.position;
						entityTransforms.rotations[i] = glm::vec4(trans.rotationAxis, trans.rotationAngle);
						entityTransforms.scales[i] = trans.scale;
					}
					//write
					else if(entityTransforms.read_write[i] == 2) {
						trans.position = entityTransforms.positions[i];
						trans.setRotation(glm::vec3(entityTransforms.rotations[i]), entityTransforms.rotations[i][3]);
						trans.scale = entityTransforms.scales[i];
					}
					entityTransforms.read_write[i] = 0;

					cont = true;
					break;
				}
			}
			if (cont)
				continue;
			//add it to the list if it wasn't found
			entityTransforms.ids.push_back(entityGuid);
			entityTransforms.names.push_back(comp.name);
			entityTransforms.positions.push_back(trans.position);
			entityTransforms.rotations.push_back(glm::vec4(trans.rotationAxis, trans.rotationAngle));
			entityTransforms.scales.push_back(trans.scale);
			entityTransforms.read_write.push_back(0);
			entityTransforms.count++;
		}

		//light space 'camera'
		glm::vec3 position = scene.GetComponent<TransformComponent>(0).getTranslation() + glm::vec3(0,30,0);
		glm::mat4 shadowP = glm::ortho(-40.f, 40.f, -40.f, 40.f, 1.f, 40.0f);
		glm::mat4 shadowV = glm::lookAt(position, position + lightDirection, glm::vec3(1, 0, 0));//glm::lookAt(-lightDirection, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		/*
		* Generate the shadow map
		*/
		glBindFramebuffer(GL_FRAMEBUFFER, gShadowBuffer);
		shadowGShader.use();
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glPolygonMode(GL_BACK, GL_FILL);
		glViewport(0, 0, 4096, 4096);
		GLuint modelUniform = glGetUniformLocation(GLuint(shadowGShader), "M");
		GLuint viewUniform = glGetUniformLocation(GLuint(shadowGShader), "V");
		GLuint perspectiveUniform = glGetUniformLocation(GLuint(shadowGShader), "P");
		//set the camera uniforms
		glUniformMatrix4fv(perspectiveUniform, 1, GL_FALSE, glm::value_ptr(shadowP));
		glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(shadowV));
		for (Guid entityGuid : ecs::EntitiesInScene<RenderModel, TransformComponent>(scene)) {
			RenderModel& comp = scene.GetComponent<RenderModel>(entityGuid);
			TransformComponent& trans = scene.GetComponent<TransformComponent>(entityGuid);

			glm::mat4 M = glm::translate(glm::mat4(1), trans.getTranslation()) * toMat4(trans.getRotation()) * glm::scale(glm::mat4(1), trans.getScale());

			//loop through each mesh in the renderComponent
			for each (Mesh mesh in comp.meshes) {
				mesh.geometry->bind();
				glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(M * mesh.localTransformation));
				glDrawElements(GL_TRIANGLES, mesh.numberOfIndicies, GL_UNSIGNED_INT, 0);
			}
		}

		/*
		* RENDER THE DEPTH, COLOR, NORMAL AND SHADOW TEXTURES 
		*/
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glViewport(0, 0, windowSize.x, windowSize.y);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(0.50f, 0.80f, 0.97f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		/*
		* render the skybox using the same framebuffer as the generative stage
		*/
		skyboxShader.use();
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);
		glBindVertexArray(skybox_vertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, skybox_vertexBuffer);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap);
		viewUniform = glGetUniformLocation(GLuint(skyboxShader), "V");
		perspectiveUniform = glGetUniformLocation(GLuint(skyboxShader), "P");
		glUniformMatrix4fv(perspectiveUniform, 1, GL_FALSE, glm::value_ptr(P));
		glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(glm::mat4(glm::mat3(V))));
		//glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(V));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);

		gShader.use();
		if (front_face || back_face) {
			glEnable(GL_CULL_FACE);
			if (front_face) {
				glCullFace(GL_FRONT);
			}
			else if (back_face) {
				glCullFace(GL_BACK);
			}
			else if (back_face && front_face) {
				glCullFace(GL_FRONT_AND_BACK);
			}
		}
		else {
			glDisable(GL_CULL_FACE);
		}
		//get uniform locations
		modelUniform = glGetUniformLocation(GLuint(gShader), "M");
		viewUniform = glGetUniformLocation(GLuint(gShader), "V");
		perspectiveUniform = glGetUniformLocation(GLuint(gShader), "P");
		GLuint cameraDirectionUniform = glGetUniformLocation(GLuint(gShader), "cameraDirection");
		GLuint normalMatUniform = glGetUniformLocation(GLuint(gShader), "normalMat");
		GLuint shaderStateUniform = glGetUniformLocation(GLuint(gShader), "shaderState");
		GLuint userColorUniform = glGetUniformLocation(GLuint(gShader), "userColor");
		GLuint lightSpaceMatixUniform = glGetUniformLocation(GLuint(gShader), "lightSpaceMatrix");
		//set the camera uniforms
		glUniformMatrix4fv(perspectiveUniform, 1, GL_FALSE, glm::value_ptr(P));
		glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(V));
		glUniformMatrix4fv(lightSpaceMatixUniform, 1, GL_FALSE, glm::value_ptr(shadowP * shadowV));


		
		

		for (Guid entityGuid : ecs::EntitiesInScene<RenderModel,TransformComponent>(scene)) {
			RenderModel& comp = scene.GetComponent<RenderModel>(entityGuid);
			TransformComponent& trans = scene.GetComponent<TransformComponent>(entityGuid);

			//properties the geometry is ALWAYS going to have
			glm::mat4 M = glm::translate(glm::mat4(1), trans.getTranslation()) * toMat4(trans.getRotation()) * glm::scale(glm::mat4(1), trans.getScale());

			//bind the light depth texture
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gLightDepth);

			//loop through each mesh in the renderComponent
			for each (Mesh mesh in comp.meshes) {
				mesh.geometry->bind();
				glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(M * mesh.localTransformation));
				glm::mat3 normalsMatrix = glm::mat3(glm::transpose(glm::inverse(M * mesh.localTransformation)));
				glUniformMatrix3fv(normalMatUniform, 1, GL_FALSE, glm::value_ptr(normalsMatrix));
				if ((mesh.properties & Mesh::meshProperties::m_hasTexture) != 0 && mesh.textureIndex != -1) {
					glActiveTexture(GL_TEXTURE1);
					comp.textures[mesh.textureIndex]->bind();
					glUniform1ui(shaderStateUniform, 1);
				}
				else {
					glUniform3fv(userColorUniform, 1, glm::value_ptr(mesh.meshColor));
					glDisableVertexAttribArray(2);
					glUniform1ui(shaderStateUniform, 0);
				}
				
				glDrawElements(GL_TRIANGLES, mesh.numberOfIndicies, GL_UNSIGNED_INT, 0);
			}
		}


		/*
		* render VFX that don't affect the shading. Things like billboards and particle effects
		* This uses the gFrameBuffer still as its render target
		* Render this to the screen
		*/
		glBindFramebuffer(GL_FRAMEBUFFER, gVFXBuffer);
		VFXshader.use();
		glBindVertexArray(billboard_vertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, billboard_vertexBuffer);
		//disable writing to the depth buffer, but still use it for culling
		//glDepthMask(false);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_CULL_FACE);
		viewUniform = glGetUniformLocation(GLuint(VFXshader), "V");
		perspectiveUniform = glGetUniformLocation(GLuint(VFXshader), "P");
		glUniformMatrix4fv(perspectiveUniform, 1, GL_FALSE, glm::value_ptr(P));
		glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(V));

		GLuint scaleUniform = glGetUniformLocation(GLuint(VFXshader), "scale");
		GLuint centrePosUniform = glGetUniformLocation(GLuint(VFXshader), "centrePos");
		GLuint cameraPositionUniform = glGetUniformLocation(GLuint(VFXshader), "cameraPos");
		GLuint lockingAxisUniform = glGetUniformLocation(GLuint(VFXshader), "lockingAxis");
		GLuint typeUniform = glGetUniformLocation(GLuint(VFXshader), "type");
		//Billboards
		glUniform1ui(typeUniform, 0);
		glUniform3fv(cameraPositionUniform, 1, glm::value_ptr(cameras[0].cameraPos));
		for (Guid entityGuid : ecs::EntitiesInScene<VFXBillboard, TransformComponent>(scene)) {
			VFXBillboard& comp = scene.GetComponent<VFXBillboard>(entityGuid);
			TransformComponent& trans = scene.GetComponent<TransformComponent>(entityGuid);

			glUniform3fv(centrePosUniform, 1, glm::value_ptr(trans.getTranslation()));
			glUniform3fv(lockingAxisUniform, 1, glm::value_ptr(glm::vec3(0, 1, 0)));
			glUniform2fv(scaleUniform, 1, glm::value_ptr(glm::vec2(trans.getScale().x, trans.getScale().y)));
			comp.texture->bind();
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		//textureStrips
		glUniform1ui(typeUniform, 1);
		glUniform3fv(centrePosUniform, 1, glm::value_ptr(glm::vec3(0, 0, 0)));
		for (Guid entityGuid : ecs::EntitiesInScene<VFXTextureStrip, TransformComponent>(scene)) {
			VFXTextureStrip& comp = scene.GetComponent<VFXTextureStrip>(entityGuid);
			TransformComponent& trans = scene.GetComponent<TransformComponent>(entityGuid);

			//glUniform3fv(centrePosUniform, 1, glm::value_ptr(trans.getTranslation()));
			

			comp.texture->bind();
			comp.GPUline->bind();
			glDisableVertexAttribArray(1);
			glDrawElements(GL_TRIANGLES, comp.indicies.size(), GL_UNSIGNED_INT, 0);
		}

		//particles
		particleShader.use();
		viewUniform = glGetUniformLocation(GLuint(particleShader), "V");
		perspectiveUniform = glGetUniformLocation(GLuint(particleShader), "P");
		cameraPositionUniform = glGetUniformLocation(GLuint(particleShader), "cameraPos");
		glUniformMatrix4fv(perspectiveUniform, 1, GL_FALSE, glm::value_ptr(P));
		glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(V));
		glUniform3fv(cameraPositionUniform, 1, glm::value_ptr(cameras[0].cameraPos));
		
		
		glBindVertexArray(particles_vertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, particles_instanceTransformBuffer);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for (Guid entityGuid : ecs::EntitiesInScene<VFXParticleSystem, TransformComponent>(scene)) {
			VFXParticleSystem& comp = scene.GetComponent<VFXParticleSystem>(entityGuid);
			TransformComponent& trans = scene.GetComponent<TransformComponent>(entityGuid);
			comp.texture->bind();
			const size_t maxBatchSize = 500;
			/*
			* render in batches of up to 500
			* start at the front index and go until either the end of the array or the max batch size. If the end is reached before the endIndex
			* then make a second call from the start of the array and work until the endIndex is reached
			*/
			size_t index = comp.frontIndex;
			size_t toRender = comp.backIndex <= comp.frontIndex && comp.particleCount != 0 ? (comp.backIndex + comp.maxParticles) - comp.frontIndex : comp.backIndex - comp.frontIndex;
			while (toRender != 0) {
				size_t rendering = comp.backIndex <= index ? comp.maxParticles - index : comp.backIndex - index;
				toRender -= rendering;
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 7 * rendering, &(comp.positions[index]), GL_DYNAMIC_DRAW);
				glDrawArraysInstanced(GL_TRIANGLES, 0, 6, rendering);
				index = index + rendering >= comp.maxParticles ? index = 0 : index = index + rendering;
			}
		}
		glDisable(GL_BLEND);
		/*
		* APPLY SHADING EFFECTS AND DRAW TO gColor buffer
		*/
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, windowSize.x, windowSize.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(true);
		celShader.use();
		glBindVertexArray(quad_vertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexBuffer);
		glDisable(GL_DEPTH_TEST);

		GLuint normWeightUniform = glGetUniformLocation(GLuint(celShader), "normalDiffWeight");
		GLuint depthWeightUniform = glGetUniformLocation(GLuint(celShader), "depthDiffWeight");

		glUniform1f(normWeightUniform, normalDiffWeight);
		glUniform1f(depthWeightUniform, depthDiffWeight);

		GLuint lightUniform = glGetUniformLocation(GLuint(celShader), "lightDir");
		GLuint ambiantUniform = glGetUniformLocation(GLuint(celShader), "ambiantStr");
		GLuint diffuseWeightUniform = glGetUniformLocation(GLuint(celShader), "diffuseWeight");
		GLuint quantizedSplitsUniform = glGetUniformLocation(GLuint(celShader), "numQuantizedSplits");
		GLuint goochWarmUniform = glGetUniformLocation(GLuint(celShader), "goochWarm");
		GLuint goochCoolUniform = glGetUniformLocation(GLuint(celShader), "goochCool");
		GLuint goochStrengthUniform = glGetUniformLocation(GLuint(celShader), "goochWeight");
		glUniform3fv(lightUniform, 1, glm::value_ptr(lightDirection));
		glUniform1f(ambiantUniform, ambiantStrength);
		glUniform1f(diffuseWeightUniform, diffuseWeight);
		glUniform1i(quantizedSplitsUniform, numQuantizedSplits);
		glm::vec3 goochWarmPass = goochWarm;
		glUniform3fv(goochWarmUniform, 1, glm::value_ptr(goochWarmPass));
		glm::vec3 goochCoolPass = goochCool;
		glUniform3fv(goochCoolUniform, 1, glm::value_ptr(goochCoolPass));
		glUniform1f(goochStrengthUniform, goochStrength);

		//bind the textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gColor);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, gDepth);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, gShadow);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, gVFXColor);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, gVFXDepth);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		

		//render line components
		//switch shader program
		lineShader.use();
		//bind uniforms
		modelUniform = glGetUniformLocation(GLuint(lineShader), "M");
		viewUniform = glGetUniformLocation(GLuint(lineShader), "V");
		perspectiveUniform = glGetUniformLocation(GLuint(lineShader), "P");
		GLuint colorUniform = glGetUniformLocation(GLuint(lineShader), "userColor");
		//set camera uniforms
		glUniformMatrix4fv(perspectiveUniform, 1, GL_FALSE, glm::value_ptr(P));
		glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(V));
		for (Guid entityGuid : ecs::EntitiesInScene<RenderLine, TransformComponent>(scene)) {
			RenderLine& comp = scene.GetComponent<RenderLine>(entityGuid);
			TransformComponent& trans = scene.GetComponent<TransformComponent>(entityGuid);
			
			glm::mat4 M = glm::translate(glm::mat4(1), trans.getTranslation()) * toMat4(trans.getRotation()) * glm::scale(glm::mat4(1), trans.getScale());
			glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(M));

			glUniform3fv(colorUniform, 1, glm::value_ptr(comp.color));
			
			comp.geometry->bind();
			glDrawArrays(GL_LINE_STRIP, 0, comp.numberOfVerticies);
		}

		
		if (showColliders) {
			//Render debug wireframes of physx colliders
			wireframeShader.use();
			modelUniform = glGetUniformLocation(GLuint(lineShader), "M");
			viewUniform = glGetUniformLocation(GLuint(lineShader), "V");
			perspectiveUniform = glGetUniformLocation(GLuint(lineShader), "P");
			glUniformMatrix4fv(perspectiveUniform, 1, GL_FALSE, glm::value_ptr(P));
			glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(V));
			physx::PxScene* physxScene;
			PxGetPhysics().getScenes(&physxScene, 1);
			int nbActors = physxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC | physx::PxActorTypeFlag::eRIGID_STATIC);
			if (nbActors)
			{
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				const int MAX_NUM_ACTOR_SHAPES = 128;
				using namespace physx;
				std::vector<PxRigidActor*> actors(nbActors);
				physxScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<PxActor**>(&actors[0]), nbActors);
				Snippets::renderActors(&actors[0], static_cast<PxU32>(actors.size()), modelUniform);
			}
		}
	}
}

// Function to return a camera view matrix (used for debug)
// TODO:: add a cameraID to pass in for multiple cameras
glm::mat4 GraphicsSystem::getCameraView() {
	return cameras[0].getView();
}

glm::vec3 GraphicsSystem::g_cameraPosition() {
	return cameras[0].cameraPos;
}

glm::vec3 GraphicsSystem::g_cameraVelocity() {
	return cameras[0].velocity;
}

void GraphicsSystem::input(SDL_Event& _event, int _cameraID)
{
	cameras[_cameraID].input(_event);
}


void GraphicsSystem::importOBJ(RenderModel& _component, const std::string _fileName) {
	std::cout << "importing " << _fileName << '\n';
	_component.name = std::string(_fileName);
	Assimp::Importer importer;
	//importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
	const aiScene* scene = importer.ReadFile("models/" + _fileName, aiProcess_Triangulate); // | aiProcess_FindInvalidData | aiProcess_FindDegenerates | aiProcess_SortByPType
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "Error importing " << _fileName << " into scene\n";
		return;
	}
	
	processNode(scene->mRootNode, scene, _component);
}


void GraphicsSystem::processNode(aiNode* node, const aiScene* scene, RenderModel& _component) {
	//process all the meshes contained in the node
	for (int m = 0; m < node->mNumMeshes; m++) {
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[m]];
		CPU_Geometry geometry;

		//process the aiMess into a CPU_Geometry to pass to the render component to create a new mesh
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			geometry.verts.push_back(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
			geometry.norms.push_back(glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
			if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
				geometry.texs.push_back(glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));

		}
		// process indices
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			geometry.indicies.push_back(face.mIndices[0]);
			geometry.indicies.push_back(face.mIndices[1]);
			geometry.indicies.push_back(face.mIndices[2]);
		}
		
		int ID = _component.attachMesh(geometry);
		_component.meshes[_component.getMeshIndex(ID)].name = mesh->mName.C_Str();
		// process material


		if (mesh->mMaterialIndex >= 0) //if the mesh has a material attached
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			aiColor3D color(0.f, 0.f, 0.f);
			material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			_component.meshes[_component.getMeshIndex(ID)].meshColor = glm::vec3(color.r, color.g, color.b);

			if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
				//get the textures name(?)
				aiString str;
				material->GetTexture(aiTextureType_DIFFUSE, 0, &str);
				std::string temp = std::string(str.C_Str());
				std::string temp2;
				for (int i = temp.size()-1; i >= 0; i--) {
					if (temp[i] != '\\')
						temp2.push_back(temp[i]);
					else
						break;
				}
				temp.clear();
				//reverse the string
				for (int i = temp2.size() - 1; i >= 0; i--) {
					temp.push_back(temp2[i]);
				}
				
				_component.attachTexture(temp, ID);
			}
		}
	}

	//process each of the nodes children
	for (int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, _component);
	}
}

/*
* importing an object geometry only
*/
void GraphicsSystem::importOBJ(CPU_Geometry& _geometry, const std::string _fileName) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("models/" + _fileName, aiProcess_Triangulate);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "Error importing " << _fileName << " into scene\n";
		return;
	}

	processNode(scene->mRootNode, scene, _geometry);
}

void GraphicsSystem::importSplineFromOBJ(CPU_Geometry& _geometry, std::string filename)
{
	std::string line;

	// Read from the text file
	std::ifstream inputfile("models/" + filename);

	// Use a while loop together with the getline() function to read the file line by line
	while (getline (inputfile, line)) {
	// Output the text from the file
	std::cout << line;

	glm::vec3 vert;

	int scanned = sscanf(line.c_str(), "v %f %f %f", &vert.x, &vert.y, &vert.z);

	if (scanned == 3)
	{
		_geometry.verts.push_back(vert);
	}


	}
	// Close the file
	inputfile.close(); 
	_geometry.verts.push_back(_geometry.verts[0]);

}


void GraphicsSystem::processNode(aiNode* node, const aiScene* scene, CPU_Geometry& _geometry) {
	//process all the meshes contained in the node
	for (int m = 0; m < node->mNumMeshes; m++) {
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[m]];

		//process the aiMess into a CPU_Geometry to pass to the render component to create a new mesh
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			_geometry.verts.push_back(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
		}
		// process indices
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			_geometry.indicies.push_back(face.mIndices[0]);
			_geometry.indicies.push_back(face.mIndices[1]);
			_geometry.indicies.push_back(face.mIndices[2]);
		}
	}

	//process each of the nodes children
	for (int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, _geometry);
	}
}
