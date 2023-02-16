#include "systems/ecs.h"
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
#include "components.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

GraphicsSystem::GraphicsSystem(Window& _window) :
	modelShader("shaders/lighting_simple.vert", "shaders/lighting_simple.frag"),
	lineShader("shaders/line.vert", "shaders/line.frag")
{
	windowSize = _window.getSize();
}

void GraphicsSystem::Update(ecs::Scene& scene, float deltaTime) {
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glClearColor(0.50f, 0.80f, 0.97f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	for (int i = 0; i < numCamerasActive; i++) {
		//matricies that need only be set once per camera
		glm::mat4 P = glm::perspective(glm::radians(45.0f), (float)windowSize.x / windowSize.y, 0.01f, 1000.f);
		glm::mat4 V = cameras[i].getView();
		// Hardcoded camera value, it can't move after this 
		
		/*
		glm::mat4 V = { 0.658686, -0.565264, 0.496598, 0,
						0, 0.660003, 0.751263, 0,
						-0.752418, -0.494847, 0.434735, 0,
						9.27202, -0.914308, -33.4781, 1
		};*/

		//set the viewport
		if (numCamerasActive <= 1) { //there can't be 0 cameras, assume always 1 minimum
			glViewport(0, 0, windowSize.x, windowSize.y);
		}
		else {
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
		}
		
		//render model components without textures
		//switch the shader program
		modelShader.use();
		//get uniform locations
		GLuint modelUniform = glGetUniformLocation(GLuint(modelShader), "M");
		GLuint viewUniform = glGetUniformLocation(GLuint(modelShader), "V");
		GLuint perspectiveUniform = glGetUniformLocation(GLuint(modelShader), "P");
		GLuint normalMatUniform = glGetUniformLocation(GLuint(modelShader), "normalMat");
		GLuint lightUniform = glGetUniformLocation(GLuint(modelShader), "light");
		GLuint viewPosUniform = glGetUniformLocation(GLuint(modelShader), "viewPos");
		GLuint ambiantStrengthUniform = glGetUniformLocation(GLuint(modelShader), "ambiantStr");
		GLuint specularStrengthUniform = glGetUniformLocation(GLuint(modelShader), "specularStrength");
		GLuint shaderStateUniform = glGetUniformLocation(GLuint(modelShader), "shaderState");
		GLuint userColorUniform = glGetUniformLocation(GLuint(modelShader), "userColor");
		//set the camera uniforms
		glUniformMatrix4fv(perspectiveUniform, 1, GL_FALSE, glm::value_ptr(P));
		glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(V));
		glUniform3fv(viewPosUniform, 1, glm::value_ptr(cameras[i].getPos()));

		//draw the models
		for (Guid entityGuid : ecs::EntitiesInScene<RenderModel,TransformComponent>(scene)) {
			RenderModel& comp = scene.GetComponent<RenderModel>(entityGuid);
			TransformComponent& trans = scene.GetComponent<TransformComponent>(entityGuid);
			
			//properties the geometry is ALWAYS going to have
			glm::mat4 M = glm::translate(glm::mat4(1), trans.getPosition()) * toMat4(trans.getRotation()) * glm::scale(glm::mat4(1), trans.getScale());
			glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(M));
			glm::mat3 normalsMatrix = glm::mat3(glm::transpose(glm::inverse(M)));
			glUniformMatrix3fv(normalMatUniform, 1, GL_FALSE, glm::value_ptr(normalsMatrix));
			//uniforms that don't change, however I put them here just in case we want to change them
			glUniform3fv(lightUniform, 1, glm::value_ptr(glm::vec3(0, 20, 0)));
			glUniform1f(ambiantStrengthUniform, 0.50f);
			glUniform1f(specularStrengthUniform, 1.5f);

			//loop through each mesh in the renderComponent
			for each (Mesh mesh in comp.meshes) {
				if ((mesh.properties & 2) != 0 && mesh.textureIndex != -1) {
					comp.textures[mesh.textureIndex]->bind();
					glUniform1ui(shaderStateUniform, 1);
				}
				else {
					glUniform3fv(userColorUniform, 1, glm::value_ptr(mesh.meshColor));
					glUniform1ui(shaderStateUniform, 0);
				}
				mesh.geometry->bind();
				glDrawElements(GL_TRIANGLES, mesh.numberOfIndicies, GL_UNSIGNED_INT, 0);
			}
		}

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
			
			glm::mat4 M = glm::translate(glm::mat4(1), trans.getPosition()) * toMat4(trans.getRotation());
			glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(M));

			glUniform3fv(colorUniform, 1, glm::value_ptr(comp.color));
			
			comp.geometry->bind();
			glDrawArrays(GL_LINE_STRIP, 0, comp.numberOfVerticies);
		}
	}
}

// Function to return a camera view matrix (used for debug)
// TODO:: add a cameraID to pass in for multiple cameras
glm::mat4 GraphicsSystem::getCameraView() {
	return cameras[0].getView();
}

void GraphicsSystem::input(SDL_Event& _event, int _cameraID)
{
	cameras[_cameraID].input(_event);
}

void GraphicsSystem::importOBJ(RenderModel& _component, const std::string _fileName) {
	std::cout << "Beginning to load model " << _fileName << "\n";
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("models/" + _fileName, aiProcess_Triangulate );
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "Error importing " << _fileName << " into scene\n";
		return;
	}
	
	processNode(scene->mRootNode, scene, _component);
}


void GraphicsSystem::processNode(aiNode* node, const aiScene* scene, RenderModel& _component) {
	std::cout << "processing node...\n";
	std::cout << "\tmeshes: " << node->mNumMeshes << '\n';
	//process all the meshes contained in the node
	for (int m = 0; m < node->mNumMeshes; m++) {
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[m]];
		CPU_Geometry geometry;

		//process the aiMess into a CPU_Geometry to pass to the render component to create a new mesh
		std::cout << "\t\tverticies: " << mesh->mNumVertices << '\n';
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			geometry.verts.push_back(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
			geometry.norms.push_back(glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
			if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
				geometry.texs.push_back(glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
		}
		// process indices
		std::cout << "\t\tfaces: " << mesh->mNumFaces << '\n';
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			geometry.indicies.push_back(face.mIndices[0]);
			geometry.indicies.push_back(face.mIndices[1]);
			geometry.indicies.push_back(face.mIndices[2]);
		}
		std::cout << "\t\tindicies: " << geometry.indicies.size() << '\n';
		// process material
		/* SAM TODO. Quite frankly this breaks my mind rn with the fact a material can have MULTIPLE textures SOMEHOW
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			vector<Texture> diffuseMaps = loadMaterialTextures(material,
				aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			vector<Texture> specularMaps = loadMaterialTextures(material,
				aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		}*/
		std::cout << "finished processing node\n";
		_component.attachMesh(geometry);
	}

	//process each of the nodes children
	for (int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, _component);
	}
}

