// HACK(beau): VS is defining this for us and complaining
// but without vs its not defined. Solin is to find a way
// to get cmake to define this regardless I think
// - Beau
#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32)
#define _DEBUG
#endif
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
