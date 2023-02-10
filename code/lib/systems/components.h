#pragma once
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

/*
* Render component is what is visible to the user
*/
struct RenderComponent
{
	GPU_Geometry* geom = new GPU_Geometry();
	int numVerts = 0;
	Texture* texture = nullptr;
	float specular = 0;
	GLuint shaderState = 0;
	//color | texture | normals | specular
	char appearance = 0; //0 = solid, 1 = wireframe
	RenderComponent() = default;
	RenderComponent(CPU_Geometry* _geom) { 
		geom->setCols(_geom->cols); 
		geom->setVerts(_geom->verts); 
		numVerts = _geom->verts.size(); 
		geom->setTexCoords(_geom->texs); 
		if (_geom->texPath.length() > 0) {
			texture = new Texture(_geom->texPath, GL_NEAREST);
			shaderState |= 2;
		}
		else if (_geom->cols.size() > 0) {
			shaderState |= 1;
		}

		if (_geom->norms.size() > 0)
			shaderState |= 4;
	}
};