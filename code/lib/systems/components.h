#pragma once
#include "../graphics/Geometry.h"
#include "../graphics/Texture.h"
#include "PxPhysicsAPI.h"
#include <glm/gtc/quaternion.hpp> 
#include <glm/gtx/quaternion.hpp>

//Helper functions that allow for easy transforms between physx and glm
inline glm::vec3 PxtoGLM(physx::PxVec3 _vec) {
	return glm::vec3(_vec.x, _vec.y, _vec.z);
}

inline glm::quat PxtoGLM(physx::PxQuat _quat) {
	return glm::quat(_quat.x, _quat.y, _quat.z, _quat.w);
}

inline physx::PxVec3 GLMtoPx(glm::vec3 _vec) {
	return physx::PxVec3(_vec.x, _vec.y, _vec.z);
}

inline physx::PxQuat GLMtoPx(glm::quat _quat) {
	return physx::PxQuat(_quat.x, _quat.y, _quat.z, _quat.w);
}

/*
* Transform describes the objects position in 3D space.
*/
struct TransformComponent
{
public:
	//constructors
	/*
	* This constructor will make the TransformComponent with a RigidActor already attached
	*/
	TransformComponent(physx::PxRigidActor* _actor) : actor(_actor) {};

	/*
	* This constructor will make the transform component with no defined actor attached.
	*/
	TransformComponent() {}

	/*
	* This constructor will make the transform component using glm transforms to initalize it and set its position
	*/
	TransformComponent(glm::vec3 _position, glm::quat _rotation) : position(_position), rotation(_rotation) {};

	//functions to maniplate the transform
	glm::vec3 getPosition() {
		if (actor == nullptr)
			return glm::vec3(position);
		else
			return PxtoGLM(actor->getGlobalPose().p);
	}
	glm::quat getRotation() {
		if (actor == nullptr)
			return glm::quat(rotation);
		else
			return PxtoGLM(actor->getGlobalPose().q);
	}

	void setPosition(glm::vec3 _position) {
		if (actor == nullptr)
			position = _position;
		else
			actor->setGlobalPose(physx::PxTransform(GLMtoPx(_position)));
	}
	void setRotation(glm::quat _rotation) {
		if (actor == nullptr)
			rotation = _rotation;
		else
			actor->setGlobalPose(physx::PxTransform(GLMtoPx(_rotation)));
	}
private:
	glm::vec3 position = glm::vec3(0);
	glm::quat rotation = glm::quat(0, 0, 0, 0);
	physx::PxRigidActor* actor = nullptr;

};

struct Mesh {
	friend class GraphicsSystem;
	unsigned int numberOfVerticies = 0;
	int numberOfIndicies = 0;
	unsigned int ID = -1;
	GPU_Geometry* geometry = new GPU_Geometry;
	int textureIndex = -1;
	glm::vec3 meshColor = glm::vec3(1);
	GLuint properties = 0; //texCoords | normals
};

/*
* Render component is what is visible to the user
*/
struct RenderModel {
public:
	//constructors
	RenderModel() = default;
	//accessors
	int g_numberOfVerts() {return numberOfVerts;};
	int g_numberOfVerts(int _meshID) { int i = getMeshIndex(_meshID); if (i != -1) { return meshes[i].numberOfVerticies; } else { return -1; } }
	bool g_hasNormals(int _meshID) { int i = getMeshIndex(_meshID); if (i != -1) { return (meshes[i].properties & 1) != 0; } else { return -1; } }
	bool g_hasTextureCoords(int _meshID) { int i = getMeshIndex(_meshID); if (i != -1) { return (meshes[i].properties & 2) != 0; } else { return -1; } }

	//modifiers
	/*
	* Attaches a mesh to the render.
	* returns: -1 if a failure, local ID of the mesh otherwise
	*/
	int attachMesh(CPU_Geometry& _geometry);
	/*
	* Attaches a texture to a mesh already in the component
	* returns: true if successful, false otherwise
	*/
	bool attachTexture(std::string _texturePath, unsigned int _meshID);
//private functions
private:
	int getMeshIndex(int _meshID) {
		for (int i = 0; i < meshes.size(); i++) {
			if (meshes[i].ID == _meshID)
				return i;
		}
		return -1;
	}

//friend fields
private:
	friend class GraphicsSystem;
	//a single render component can have multiple meshes and corresponding textures attached
	std::vector<Mesh> meshes;	
	std::vector<Texture*> textures;	
//private fields
private:
	unsigned int currentMeshID = 0;
	int numberOfVerts = 0;
};

struct RenderLine {
	RenderLine() = default;
	RenderLine(const CPU_Geometry _geometry);
	void setColor(const glm::vec3 _color);
private:
	friend class GraphicsSystem;
	GPU_Geometry* geometry = new GPU_Geometry();
	glm::vec3(color) = glm::vec3(1);
	GLuint numberOfVerticies = 0;
};
