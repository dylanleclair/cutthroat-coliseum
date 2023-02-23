#pragma once
// #include "GLM/glm.h"
#include <glm/glm.hpp>
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