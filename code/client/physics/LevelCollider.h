#pragma once
#include "PxPhysicsAPI.h"
#include <vector>
#include <iostream>
#include "systems/GraphicsSystem.h"
#include <string>
#include "../systems/PhysicsSystem.h"

using namespace physics;

struct LevelCollider
{ 
    LevelCollider(std::string modelName, PhysicsSystem& physics);
    LevelCollider(CPU_Geometry& levelGeom, PhysicsSystem& physics);

    physx::PxTriangleMesh * LevelCollider::cookLevel(); 
    void LevelCollider::initLevelRigidBody(physx::PxTriangleMesh* levelMesh);


    // physx::PxTriangleMesh * cookLevel(std::vector<physx::PxVec3> levelVertices, std::vector<physx::PxU32> levelIndices, physx::PxPhysics* gPhysics, physx::PxCooking *gCooking);
    // void LevelCollider::initLevelRigidBody(physx::PxPhysics* gPhysics, physx::PxScene* scene, physx::PxMaterial* material, physx::PxTriangleMesh* levelMesh);


private:
    std::vector<physx::PxVec3> levelVertices = std::vector<physx::PxVec3>();
    std::vector<physx::PxU32> levelIndices = std::vector<physx::PxU32>();
    PhysicsSystem& m_physicsSystem;
};
