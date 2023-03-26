#pragma once
#include "PxPhysicsAPI.h"
#include <vector>
#include <iostream>
#include "../../systems/GraphicsSystem.h"
#include <string>
#include "../../systems/PhysicsSystem.h"

using namespace physics;

struct LevelCollider
{ 
    physx::PxTriangleMesh * LevelCollider::cookLevel(glm::mat4 transform); 
    void LevelCollider::initLevelRigidBody(physx::PxTriangleMesh* levelMesh);


    // physx::PxTriangleMesh * cookLevel(std::vector<physx::PxVec3> levelVertices, std::vector<physx::PxU32> levelIndices, physx::PxPhysics* gPhysics, physx::PxCooking *gCooking);
    // void LevelCollider::initLevelRigidBody(physx::PxPhysics* gPhysics, physx::PxScene* scene, physx::PxMaterial* material, physx::PxTriangleMesh* levelMesh);

    void Initialize(CPU_Geometry& levelGeom, PhysicsSystem& physics);
    void Initialize(std::string modelName, PhysicsSystem& physics);

    void LevelCollider::release();
    void LevelCollider::initLevelRigidBody(physx::PxTriangleMesh* levelMesh, physx::PxMaterial* material);
    PxRigidActor* getActor() {return m_level; };
    PxShape* getShape() {return m_levelShape; };


private:
    std::vector<physx::PxVec3> levelVertices = std::vector<physx::PxVec3>();
    std::vector<physx::PxU32> levelIndices = std::vector<physx::PxU32>();
    PhysicsSystem* m_physicsSystem;

    physx::PxShape* m_levelShape;
    physx::PxRigidStatic* m_level;

};
