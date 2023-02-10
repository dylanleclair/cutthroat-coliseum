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

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

GraphicsSystem::GraphicsSystem(Window& _window) :
	shader("shaders/test.vert", "shaders/test.frag")
{
	GLDebug::enable();
	// SHADERS
	shader.use();

	windowSize = _window.getSize();

	//get uniform locations
	modelUniform = glGetUniformLocation(GLuint(shader), "M");
	viewUniform = glGetUniformLocation(GLuint(shader), "V");
	perspectiveUniform = glGetUniformLocation(GLuint(shader), "P"); 
	shaderSelectorUniform = glGetUniformLocation(GLuint(shader), "selector");
	textureUniform = glGetUniformLocation(GLuint(shader), "tex");
}

void GraphicsSystem::Update(ecs::Scene& scene, float deltaTime) {
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < numCamerasActive; i++) {
		shader.use();
		//matricies that need only be set once per camera
		glm::mat4 P = glm::perspective(glm::radians(45.0f), (float)windowSize.x/ windowSize.y, 0.01f, 1000.f);
		//glm::mat4 V = cameras[i].getView();
		// Hardcoded camera value, it can't move after this 
		glm::mat4 V = { 0.236837, -0.415199, 0.878361, 0,
						0, 0.904083, 0.427358, 0,
						- 0.971549, -0.101214, 0.21412, 0,
						0.0948601, -1.58747, -22.4296, 1 };
		glUniformMatrix4fv(perspectiveUniform, 1, GL_FALSE, glm::value_ptr(P));
		glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(V));

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
		
		//render dynamic components
		for (Guid entityGuid : ecs::EntitiesInScene<RenderComponent,TransformComponent>(scene)) {
			RenderComponent& comp = scene.GetComponent<RenderComponent>(entityGuid);
			TransformComponent& trans = scene.GetComponent<TransformComponent>(entityGuid);
			
			if(comp.appearance == 1)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			glUniform1i(shaderSelectorUniform, comp.shaderState);//0 = position and color, 1 = position and texture
			if (comp.shaderState == 1)
				comp.texture->bind();

			// GEOMETRY
			comp.geom->bind();

			glm::mat4 M = glm::translate(glm::mat4(1), trans.getPosition()) * toMat4(trans.getRotation());
			glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(M));
			glDrawArrays(GL_TRIANGLES, 0, comp.numVerts);
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

void GraphicsSystem::readVertsFromFile(RenderComponent& _component, const std::string _file, const std::string _textureFile) {
	CPU_Geometry geom;
	std::cout << "Beginning to load model\n";
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(_file, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "Error importing " << _file << " into scene\n";
		return;
	}
	processNode(scene->mRootNode, scene, &geom);
	std::cout << "Finished loading model with " << geom.verts.size() << " verticies\n";

	//Load the verticies into the GPU
	if (_textureFile.size() > 0) {
		_component.texture = new Texture(_textureFile, GL_NEAREST);
		_component.shaderState = 1;
	}
	else {
		_component.shaderState = 0;
	}
	_component.numVerts = geom.verts.size();
	_component.geom->setVerts(geom.verts);
	_component.geom->setCols(geom.cols);
	_component.geom->setTexCoords(geom.texs);
}

void GraphicsSystem::processNode(aiNode* node, const aiScene* scene, CPU_Geometry* geom) {
	for (int i = 0; i < node->mNumMeshes; i++) {
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		//for each mess extract its verticies
		//retrieve all the verticies
		std::vector<glm::vec3> tverts; //a tempoary vector to store nodes until they can be unindexed
		std::vector<glm::vec2> ttexs;
		tverts.reserve(mesh->mNumVertices);
		for (int j = 0; j < mesh->mNumVertices; j++) {
			tverts.push_back(glm::vec3(mesh->mVertices[j].x, -mesh->mVertices[j].y, mesh->mVertices[j].z));
			if (mesh->mTextureCoords[0]) {
				ttexs.push_back(glm::vec2(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y));
			}
		}
		
		//retrieve all the indicies
		//remove the dependency for indicies and store the final result
		for (int j = 0; j < mesh->mNumFaces; j++) {
			geom->verts.push_back(tverts[mesh->mFaces[j].mIndices[0]]);
			geom->verts.push_back(tverts[mesh->mFaces[j].mIndices[1]]);
			geom->verts.push_back(tverts[mesh->mFaces[j].mIndices[2]]);
			if (mesh->mTextureCoords[0]) {
				//push back the texture coordinates
				geom->texs.push_back(ttexs[mesh->mFaces[j].mIndices[0]]);
				geom->texs.push_back(ttexs[mesh->mFaces[j].mIndices[1]]);
				geom->texs.push_back(ttexs[mesh->mFaces[j].mIndices[2]]);
			}
			for (int z = 0; z < 3; z++)
				geom->cols.push_back(glm::vec3(1));
		}
		
	}

	//process each of the nodes children
	for (int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, geom);
	}
}

