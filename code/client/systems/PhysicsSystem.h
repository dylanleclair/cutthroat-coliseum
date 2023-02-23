#pragma once

#include <ctype.h>
#include <iostream>

#include "systems/ecs.h"
#include "utils/Time.h"
#include "SDL.h"
#include <limits>

#include "PxPhysicsAPI.h"
#include "../physx/snippetcommon/SnippetPVD.h"

// VEHICLES
#include "vehicle2/PxVehicleAPI.h"
#include "../physx/snippetvehicle2common/enginedrivetrain/EngineDrivetrain.h"
#include "../physx/snippetvehicle2common/serialization/BaseSerialization.h"
#include "../physx/snippetvehicle2common/serialization/EngineDrivetrainSerialization.h"
#include "../physx/snippetvehicle2common/SnippetVehicleHelpers.h"

using namespace physx;
using namespace physx::vehicle2;
using namespace snippetvehicle2;

namespace physics {

struct PhysicsSystem : ecs::ISystem
{
	// PhysX management class instances.
	PxDefaultAllocator m_Allocator;
	PxDefaultErrorCallback m_ErrorCallback;
	PxFoundation* m_Foundation;
	PxPhysics* m_Physics;
	PxDefaultCpuDispatcher* m_Dispatcher;
	PxScene* m_Scene;
	PxMaterial* m_Material;
	PxPvd* m_Pvd;

    // The mapping between PxMaterial and friction.
    PxVehiclePhysXMaterialFriction m_PhysXMaterialFrictions[16];
    PxU32 m_NbPhysXMaterialFrictions;
    PxReal m_PhysXDefaultMaterialFriction;

    // A ground plane to drive on.
    PxRigidStatic* gGroundPlane;

	// Gravitational acceleration
	const PxVec3 m_Gravity{0.0f, -9.81f, 0.0f};

    void Update(ecs::Scene& scene, Timestep timestep);
	void Update(ecs::Scene& scene, float deltaTime);


	void Initialize();
	void Cleanup();
private: 
	void initPhysX();
	void cleanupPhysX();

	void initGroundPlane();
	void cleanupGroundPlane();

	void initMaterialFrictionTable();

};





}