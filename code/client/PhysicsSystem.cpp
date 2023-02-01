#include "PhysicsSystem.h"


void PhysicsSystem::Update(ecs::Scene& scene, float deltaTime)
{
	//update the scene
	gScene->simulate(deltaTime);
	gScene->fetchResults(true);

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
	physx::PxTransform tran(physx::PxVec3(0,40,0)); //put the cube 40 units in the air
	body = physSystem.gPhysics->createRigidDynamic(tran); 

	physx::PxShape* shape = physSystem.gPhysics->createShape(physx::PxBoxGeometry(halfLen, halfLen, halfLen), *physSystem.gMaterial);
	body->attachShape(*shape);

	physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
	physSystem.gScene->addActor(*body);
}