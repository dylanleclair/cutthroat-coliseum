#include "systems/ecs.h"
#include "GraphicsSystem.h"

#include <GL/glew.h>
#include <iostream>
#include "GLDebug.h"
#include "ShaderProgram.h"
#include "glm/gtc/type_ptr.hpp"
#include "Camera.h"
#include "Position.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

GraphicsSystem::GraphicsSystem(Window& _window) :
	shader("shaders/test.vert", "shaders/test.frag")
{
	// SHADERS
	shader.use();

	windowSize = _window.getSize();

	//get uniform locations
	modelUniform = glGetUniformLocation(GLuint(shader), "M");
	viewUniform = glGetUniformLocation(GLuint(shader), "V");
	perspectiveUniform = glGetUniformLocation(GLuint(shader), "P");
}

void GraphicsSystem::Update(ecs::Scene& scene, float deltaTime) {
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	for (int i = 0; i < numCamerasActive; i++) {
		shader.use();
		//matricies that need only be set once per camera
		glm::mat4 P = glm::perspective(glm::radians(45.0f), (float)windowSize.x/ windowSize.y, 0.01f, 1000.f);
		glm::mat4 V = cameras[i].getView();
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
		std::cout << "HELLO\n";
		//render dynamic components
		for (Guid entityGuid : ecs::EntitiesInScene<RenderComponent>(scene)) {
			RenderComponent& comp = scene.GetComponent<RenderComponent>(entityGuid);

			// GEOMETRY
			GPU_Geometry gpuGeom;

			gpuGeom.bind();
			gpuGeom.setVerts(comp.geom->verts);
			gpuGeom.setCols(comp.geom->cols);

			glm::mat4 M = comp.position->getTransformMatrix();
			glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(M));
			glDrawArrays(GL_TRIANGLES, 0, comp.geom->verts.size());
		}
	}
	
	//render dynamic components
	for (Guid entityGuid : ecs::EntitiesInScene<MeshComponent>(scene)) {
		MeshComponent& comp = scene.GetComponent<MeshComponent>(entityGuid);

		// GEOMETRY
		GPU_Geometry gpuGeom;

		gpuGeom.bind();
		gpuGeom.setVerts(comp.geom->verts);
		gpuGeom.setCols(comp.geom->cols);

		glm::mat4 M = glm::mat4(1);
		glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(M));
		glDrawArrays(GL_TRIANGLES, 0, comp.geom->verts.size());
	}
}

void GraphicsSystem::input(SDL_Event& _event, int _cameraID)
{
	cameras[_cameraID].input(_event);
}

MeshComponent::MeshComponent(std::string _file)
{
	std::cout << "Beginning to load model\n";
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(_file, aiProcess_Triangulate);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "Error importing " << _file << " into scene\n";
		return;
	}
	std::cout << "Found " << scene->mRootNode->mNumMeshes << " Meshes\n";
	std::cout << "root node contains " << scene->mRootNode->mNumChildren << " Children\n";
	processNode(scene->mRootNode, scene, geom);
	std::cout << "Finished loading model with " << geom->verts.size() << " verticies\n";
}

void MeshComponent::processNode(aiNode* node, const aiScene* scene, CPU_Geometry* geom) {
	//process (For now I'm only processing the root node)
	for (int i = 0; i < node->mNumMeshes; i++) {
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		//for each mess extract its verticies
		//retrieve all the verticies
		std::vector<glm::vec3> tverts;
		tverts.reserve(mesh->mNumVertices);
		for (int j = 0; j < mesh->mNumVertices; j++) {
			tverts.push_back(glm::vec3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z));
		}
		std::cout << "Finished loading verticies\n";
		//retrieve all the indicies
		//remove the dependency for indicies and store the final result
		for (int j = 0; j < mesh->mNumFaces; j++) {
			geom->verts.push_back(tverts[mesh->mFaces[j].mIndices[0]]);
			geom->verts.push_back(tverts[mesh->mFaces[j].mIndices[1]]);
			geom->verts.push_back(tverts[mesh->mFaces[j].mIndices[2]]);
			for (int z = 0; z < 3; z++)
				geom->cols.push_back(glm::vec3(1));
		}
	}

	for (int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, geom);
	}
}