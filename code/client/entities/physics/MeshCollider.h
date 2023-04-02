#pragma once
#include "PxPhysicsAPI.h"
#include <vector>
#include <iostream>
#include "../../systems/GraphicsSystem.h"
#include <string>
#include "../../systems/PhysicsSystem.h"

using namespace physics;

struct MeshCollider
{
  physx::PxTriangleMesh *MeshCollider::cookLevel(glm::mat4 transform);
  void MeshCollider::initLevelRigidBody(physx::PxTriangleMesh *levelMesh);

  // physx::PxTriangleMesh * cookLevel(std::vector<physx::PxVec3> levelVertices, std::vector<physx::PxU32> levelIndices, physx::PxPhysics* gPhysics, physx::PxCooking *gCooking);
  // void MeshCollider::initLevelRigidBody(physx::PxPhysics* gPhysics, physx::PxScene* scene, physx::PxMaterial* material, physx::PxTriangleMesh* levelMesh);

  void Initialize(CPU_Geometry &levelGeom, PhysicsSystem &physics);
  void Initialize(std::string modelName, PhysicsSystem &physics);

  void MeshCollider::release();
  void MeshCollider::initLevelRigidBody(physx::PxTriangleMesh *levelMesh, physx::PxMaterial *material);
  PxRigidActor *getActor() { return m_level; };
  PxShape *getShape() { return m_levelShape; };

private:
  std::vector<physx::PxVec3> levelVertices = std::vector<physx::PxVec3>();
  std::vector<physx::PxU32> levelIndices = std::vector<physx::PxU32>();
  PhysicsSystem *m_physicsSystem;

  physx::PxShape *m_levelShape;
  physx::PxRigidStatic *m_level;
};


void spawnMeshCollider(std::string modelName, ecs::Scene& scene, PhysicsSystem& ps, PxMaterial* material);