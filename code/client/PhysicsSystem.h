#pragma once
#include "GraphicsSystem.h"
#include "PxPhysicsAPI.h"

struct RigidbodyComponent
{
	physx::PxRigidDynamic* body = nullptr;
	RigidbodyComponent() = default;
	void intalize(const PhysicsSystem& physSystem);
};

struct PhysicsSystem : ecs::ISystem {
	physx::PxDefaultAllocator      gAllocator;
	physx::PxDefaultErrorCallback  gErrorCallback;
	physx::PxFoundation* gFoundation;
	physx::PxPhysics* gPhysics;
	physx::PxDefaultCpuDispatcher* gDispatcher;
	physx::PxScene* gScene;
	physx::PxMaterial* gMaterial;
	physx::PxPvd* gPvd;
public:
	void Update(ecs::Scene& scene, float deltaTime);
private:

};