/*
#include "PhysicsSystem.h"
#include "GraphicsSystem.h"
#include <systems/ecs.h>

#define PVD_HOST "127.0.0.1"

PhysicsSystem::PhysicsSystem()
{
	using namespace physx;
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	gScene = gPhysics->createScene(sceneDesc);
	
	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
	gScene->addActor(*groundPlane);
}

void PhysicsSystem::Update(ecs::Scene& scene, float deltaTime)
{

	//update the scene
	std::cout << "simulating scene with " << deltaTime << " milliseconds\n";
	gScene->simulate(deltaTime);
	std::cout << "finished simulating scene\n";
	gScene->fetchResults(true);
	std::cout << "fetched results\n";
	

	//update the transform components
	for (Guid entityGuid : ecs::EntitiesInScene<RigidbodyComponent>(scene)) {
		//get the 2 components needed
		RigidbodyComponent& rbComp = scene.GetComponent<RigidbodyComponent>(entityGuid);
		TransformComponent& transComp = scene.GetComponent<TransformComponent>(entityGuid);

		//update the components
		transComp.position.x = rbComp.body->getGlobalPose().p.x; 
		transComp.position.y = rbComp.body->getGlobalPose().p.y;
		transComp.position.z = rbComp.body->getGlobalPose().p.z;
		
		transComp.rotation.x = rbComp.body->getGlobalPose().q.x;
		transComp.rotation.y = rbComp.body->getGlobalPose().q.y;
		transComp.rotation.z = rbComp.body->getGlobalPose().q.z;
		transComp.rotation.w = rbComp.body->getGlobalPose().q.w;
	}
}

void RigidbodyComponent::intalize(const PhysicsSystem& physSystem)
{
	float halfLen = 0.5f;
	physx::PxTransform tran(physx::PxVec3(0,50,-30)); //put the cube 40 units in the air
	body = physSystem.gPhysics->createRigidDynamic(tran); 

	physx::PxShape* shape = physSystem.gPhysics->createShape(physx::PxBoxGeometry(halfLen, halfLen, halfLen), *physSystem.gMaterial);
	body->attachShape(*shape);

	physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
	physSystem.gScene->addActor(*body);
}*/