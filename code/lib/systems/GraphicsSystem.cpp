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
#include "components.h"

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
	//shaderSelectorUniform = glGetUniformLocation(GLuint(shader), "selector");
	//textureUniform = glGetUniformLocation(GLuint(shader), "tex");
	//normalMatUniform = glGetUniformLocation(GLuint(shader), "normalMat");
	//lightUniform = glGetUniformLocation(GLuint(shader), "light");
	//viewPosUniform = glGetUniformLocation(GLuint(shader), "viewPos");
	//ambiantStrengthUniform = glGetUniformLocation(GLuint(shader), "ambiantStr");
	//specularStrengthUniform = glGetUniformLocation(GLuint(shader), "specularStrength");
	//colorUniform = glGetUniformLocation(GLuint(shader), "userColor");
}

void GraphicsSystem::Update(ecs::Scene& scene, float deltaTime) {
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glClearColor(0.50f, 0.80f, 0.97f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	//glEnable(GL_DEPTH_TEST);

	for (int i = 0; i < numCamerasActive; i++) {
		shader.use();
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
		

		glUniformMatrix4fv(perspectiveUniform, 1, GL_FALSE, glm::value_ptr(P));
		glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(V));
		//glUniform3fv(viewPosUniform, 1, glm::value_ptr(cameras[i].getPos()));

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
			//glUniform3fv(lightUniform, 1, glm::value_ptr(glm::vec3(0, 20, 0)));
			//glUniform1f(ambiantStrengthUniform, 0.50f);

			//loop through each mesh in the renderComponent
			for each (Mesh mesh in comp.meshes) {
				mesh.geometry->bind();
				glDrawElements(GL_TRIANGLES, mesh.numberOfIndicies, GL_UNSIGNED_INT, 0);
			}
			/*
			if(comp.appearance == 1)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			glUniform1ui(shaderSelectorUniform, 0);//comp.shaderState);

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
			*/
			// GEOMETRY
			//comp.geometry->bind();
			
			//if (comp.appearance == 2)
			//{
				//glDrawArrays(GL_LINE_STRIP, 0, comp.numberOfVerts);
			//}
			//else {
			
			//}
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

void GraphicsSystem::importOBJ(RenderComponent& _component, const std::string _fileName) {
	std::cout << "Beginning to load model " << _fileName << "\n";
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("models/" + _fileName, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "Error importing " << _fileName << " into scene\n";
		return;
	}
	
	processNode(scene->mRootNode, scene, _component);
}


void GraphicsSystem::processNode(aiNode* node, const aiScene* scene, RenderComponent& _component) {
	std::cout << "processing node...\n";
	std::cout << "\tmeshes: " << node->mNumMeshes << '\n';
	//process all the meshes contained in the node
	for (int i = 0; i < node->mNumMeshes; i++) {
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
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
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				geometry.indicies.push_back(face.mIndices[j]);
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

