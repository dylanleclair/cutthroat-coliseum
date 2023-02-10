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
	// SHADERS
	shader.use();

	windowSize = _window.getSize();

	//get uniform locations
	modelUniform = glGetUniformLocation(GLuint(shader), "M");
	viewUniform = glGetUniformLocation(GLuint(shader), "V");
	perspectiveUniform = glGetUniformLocation(GLuint(shader), "P"); 
	shaderSelectorUniform = glGetUniformLocation(GLuint(shader), "selector");
	textureUniform = glGetUniformLocation(GLuint(shader), "tex");
	normalMatUniform = glGetUniformLocation(GLuint(shader), "normalMat");
	lightUniform = glGetUniformLocation(GLuint(shader), "light");
	viewPosUniform = glGetUniformLocation(GLuint(shader), "viewPos");
	ambiantStrengthUniform = glGetUniformLocation(GLuint(shader), "ambiantStr");
	specularStrengthUniform = glGetUniformLocation(GLuint(shader), "specularStrength");
	colorUniform = glGetUniformLocation(GLuint(shader), "userColor");
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
		shader.use();
		//matricies that need only be set once per camera
		glm::mat4 P = glm::perspective(glm::radians(45.0f), (float)windowSize.x/ windowSize.y, 0.1f, 1000.f);
		glm::mat4 V = cameras[i].getView();
		glUniformMatrix4fv(perspectiveUniform, 1, GL_FALSE, glm::value_ptr(P));
		glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(V));
		glUniform3fv(viewPosUniform, 1, glm::value_ptr(cameras[i].getPos()));

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
			
			//properties the geometry is ALWAYS going to have
			glm::mat4 M = glm::translate(glm::mat4(1), trans.getPosition()) * toMat4(trans.getRotation());
			glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(M));
			//uniforms that don't change, however I put them here just in case we want to change them
			glUniform3fv(lightUniform, 1, glm::value_ptr(glm::vec3(0, 20, 0)));
			glUniform1f(ambiantStrengthUniform, 0.50f);

			if(comp.appearance == 1)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			glUniform1ui(shaderSelectorUniform, comp.shaderState);

			if ((comp.shaderState & 1) != 0)
				glUniform3fv(colorUniform, 1, glm::value_ptr(comp.color));

			//if there is an attached texture
			if ((comp.shaderState & 2) != 0)
				comp.texture->bind();

			//if specular shading is enabled
			if((comp.shaderState & 8) != 0)
				glUniform1f(specularStrengthUniform, comp.specular);

			//if the model has normals
			if ((comp.shaderState & 4) != 0) {
				glm::mat3 normalsMatrix = glm::mat3(glm::transpose(glm::inverse(M)));
				glUniformMatrix3fv(normalMatUniform, 1, GL_FALSE, glm::value_ptr(normalsMatrix));
			}

			// GEOMETRY
			comp.geom->bind();
			
			if (comp.appearance == 2)
			{
				glDrawArrays(GL_LINE_STRIP, 0, comp.numVerts);
			} else {
				glDrawArrays(GL_TRIANGLES, 0, comp.numVerts);
			}
		}
	}
}

void GraphicsSystem::input(SDL_Event& _event, int _cameraID)
{
	cameras[_cameraID].input(_event);
}

void GraphicsSystem::readVertsFromFile(RenderComponent& _component, const std::string _file, const std::string _textureFile) {
	CPU_Geometry geom;
	std::cout << "Beginning to load model " << _file << "\n";
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(_file, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "Error importing " << _file << " into scene\n";
		return;
	}
	processNode(scene->mRootNode, scene, &geom);

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
	_component.geom->setNorms(geom.norms);
	std::cout << "model has:\n";
	std::cout << '\t' << geom.verts.size() << " verticies\n";
	std::cout << '\t' << geom.verts.size() << " colors\n";
	std::cout << '\t' << geom.verts.size() << " texture coords\n";
	std::cout << '\t' << geom.verts.size() << " normals\n";
	std::cout << '\n';
}

void GraphicsSystem::processNode(aiNode* node, const aiScene* scene, CPU_Geometry* geom) {
	for (int i = 0; i < node->mNumMeshes; i++) {
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		//for each mess extract its verticies
		//retrieve all the verticies
		std::vector<glm::vec3> tverts; //a tempoary vector to store nodes until they can be unindexed
		std::vector<glm::vec2> ttexs;
		std::vector<glm::vec3> tnorms;
		tverts.reserve(mesh->mNumVertices);
		for (int j = 0; j < mesh->mNumVertices; j++) {
			tverts.push_back(glm::vec3(mesh->mVertices[j].x, -mesh->mVertices[j].y, mesh->mVertices[j].z));
			if (mesh->mTextureCoords[0]) {
				ttexs.push_back(glm::vec2(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y));
			}
			if (mesh->HasNormals())
				tnorms.push_back(-glm::vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z));
		}
		
		//retrieve all the indicies
		//remove the dependency for indicies and store the final result
		for (int j = 0; j < mesh->mNumFaces; j++) {
			geom->verts.push_back(tverts[mesh->mFaces[j].mIndices[2]]);
			geom->verts.push_back(tverts[mesh->mFaces[j].mIndices[1]]);
			geom->verts.push_back(tverts[mesh->mFaces[j].mIndices[0]]);
			if (mesh->mTextureCoords[0]) {
				//push back the texture coordinates
				geom->texs.push_back(ttexs[mesh->mFaces[j].mIndices[2]]);
				geom->texs.push_back(ttexs[mesh->mFaces[j].mIndices[1]]);
				geom->texs.push_back(ttexs[mesh->mFaces[j].mIndices[0]]);
			}
			if (mesh->HasNormals()) {
				geom->norms.push_back(tnorms[mesh->mFaces[j].mIndices[2]]);
				geom->norms.push_back(tnorms[mesh->mFaces[j].mIndices[1]]);
				geom->norms.push_back(tnorms[mesh->mFaces[j].mIndices[0]]);
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

