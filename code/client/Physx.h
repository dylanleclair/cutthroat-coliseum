#define _DEBUG
#include "PxPhysicsAPI.h"

extern physx::PxDefaultAllocator      gAllocator;
extern physx::PxDefaultErrorCallback  gErrorCallback;
extern physx::PxFoundation*           gFoundation;
extern physx::PxPhysics*              gPhysics;
extern physx::PxDefaultCpuDispatcher* gDispatcher;
extern physx::PxScene*                gScene;
extern physx::PxMaterial*             gMaterial;
extern physx::PxPvd*                  gPvd;

void init_physx();
