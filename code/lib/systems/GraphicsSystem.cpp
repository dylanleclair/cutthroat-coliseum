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
#include <PxPhysicsAPI.h>

//DEBUG IMPORTS
#include "../lib/graphics/snippetrender/SnippetRender.h"

GraphicsSystem::GraphicsSystem(Window& _window) :
	modelShader("shaders/lighting_simple.vert", "shaders/lighting_simple.frag"),
	lineShader("shaders/line.vert", "shaders/line.frag"),
	wireframeShader("shaders/wireframe.vert", "shaders/wireframe.frag")
{
	windowSize = _window.getSize();
}

// Panel to controls the cameras
void GraphicsSystem::ImGuiPanel() {
	ImGui::Begin("Camera States");

	if (ImGui::Button("Free Camera")) {
		cam_mode = 1;
	}
	if (ImGui::Button("Fixed Camera")) {
		cam_mode = 2;
	}
	if (ImGui::Button("Follow Camera")) {
		cam_mode = 3;
	}

	ImGui::End();

	ImGui::Begin("Debug Rendering");
	ImGui::Checkbox("Collider Meshes", &showColliders);
	ImGui::End();

	ImGui::Begin("Debug Rendering");
	//show all renderables in a list
	static int item_current_idx = 0;
	if (ImGui::BeginCombo("Transforms", entityTransforms.names[item_current_idx].c_str())) {
		for (int i = 0; i < entityTransforms.count; i++) {
			const bool is_selected = (item_current_idx == i);
			if (ImGui::Selectable(entityTransforms.names[i].c_str(), is_selected))
				item_current_idx = i;
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::InputFloat3("position", &(entityTransforms.positions[item_current_idx].x));
	ImGui::InputFloat4("rotation", &(entityTransforms.rotations[item_current_idx].x));
	ImGui::InputFloat3("scale", &(entityTransforms.scales[item_current_idx].x));
	
	
	ImGui::End();
}

void GraphicsSystem::Update(ecs::Scene& scene, float deltaTime) {
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glClearColor(0.50f, 0.80f, 0.97f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	for (int i = 0; i < numCamerasActive; i++) {
		//matricies that need only be set once per camera
		glm::mat4 P = glm::perspective(glm::radians(45.0f), (float)windowSize.x / windowSize.y, 0.01f, 1000.f);
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
			//calculate where the camera should aim to be positioned
			glm::vec3 cameraTargetLocation = glm::translate(glm::mat4(1), trans.getPosition()) * toMat4(trans.getRotation()) * glm::vec4(0, 2, -4, 1);
			//calculate the speed of the car
			float speed = glm::distance(previousCarPosition, trans.getPosition());
			//calculate how far the camera is from the target position
			float cameraOffset = glm::distance(cameraTargetLocation, cameras[0].getPos());
			//use a sigmoid function to determine how much to move the camera to the target position (can't go higher than 1)
			float correctionAmount = cameraOffset / (40 + cameraOffset);
			//lerp between the 2 positions according to the correction amount
			previousCarPosition = trans.getPosition();
			//lerp the camera to a good location based on the correction amount
			cameras[0].setPos(cameras[0].getPos() * (1-correctionAmount) + correctionAmount * cameraTargetLocation);
			V = glm::lookAt(cameras[0].getPos(), trans.getPosition(), glm::vec3(0.0f, 1.0f, 0.0f));
		}

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

			//search if it is already in the debug list, and if it is update it
			for (int i = 0; i < entityTransforms.count; i++) {
				if (entityTransforms.ids[i] == entityGuid) {
					trans.setPosition(entityTransforms.positions[i]);
					trans.setRotation(entityTransforms.rotations[i]);
					trans.setScale(entityTransforms.scales[i]);
					goto end;
				}
			}
			//add it to the list if it wasn't found
			entityTransforms.ids.push_back(entityGuid);
			entityTransforms.names.push_back(comp.name);
			entityTransforms.positions.push_back(trans.position);
			entityTransforms.rotations.push_back(trans.rotation);
			entityTransforms.scales.push_back(trans.scale);
			entityTransforms.count++;
end:
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

void GraphicsSystem::input(SDL_Event& _event, int _cameraID)
{
	cameras[_cameraID].input(_event);
}

void GraphicsSystem::importOBJ(RenderModel& _component, const std::string _fileName) {
	std::cout << "Beginning to load model " << _fileName << "\n";
	_component.name = std::string(_fileName);
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
		std::cout << "finished processing node\n";
		int ID = _component.attachMesh(geometry);
		// process material
		//SAM TODO. Quite frankly this breaks my mind rn with the fact a material can have MULTIPLE textures SOMEHOW
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			//std::cout << "HELLO           " << material->GetTextureCount(aiTextureType_DIFFUSE) << '\n';
			if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
				//get the textures name(?)
				aiString str;
				material->GetTexture(aiTextureType_DIFFUSE, 0, &str);
				std::cout << "Material has a texture " << str.C_Str() << '\n';
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


void GraphicsSystem::importOBJ(CPU_Geometry& _geometry, const std::string _fileName) {
	std::cout << "Beginning to load model " << _fileName << "\n";
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("models/" + _fileName, aiProcess_Triangulate);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "Error importing " << _fileName << " into scene\n";
		return;
	}

	processNode(scene->mRootNode, scene, _geometry);
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
