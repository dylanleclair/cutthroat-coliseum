#include "PxPhysicsAPI.h"
#include "MeshCollider.h"
#include <vector>
#include <iostream>
#include "../../systems/GraphicsSystem.h"
#include <string>

void MeshCollider::Initialize(std::string modelName, PhysicsSystem &physics)
{

  m_physicsSystem = &physics;
  CPU_Geometry levelGeom{};
  GraphicsSystem::importOBJ(levelGeom, modelName);
  // convert the verts / indices
  for (auto vert : levelGeom.verts)
  {
    levelVertices.push_back(GLMtoPx(vert));
  }

  for (auto &index : levelGeom.indicies)
  {
    levelIndices.push_back(index);
  }
}

void MeshCollider::Initialize(CPU_Geometry &levelGeom, PhysicsSystem &physics)
{
  // passes the geom
  m_physicsSystem = &physics;
  for (auto vert : levelGeom.verts)
  {
    levelVertices.push_back(GLMtoPx(vert));
  }

  for (auto &index : levelGeom.indicies)
  {
    levelIndices.push_back(index);
  }
}

physx::PxTriangleMesh *MeshCollider::cookLevel(glm::mat4 transform)
{
  auto gCooking = m_physicsSystem->m_Cooking;
  auto gPhysics = m_physicsSystem->m_Physics;

  for (auto &vert : levelVertices)
  {
    vert = GLMtoPx(glm::vec4{PxtoGLM(vert), 1.f} * transform);
  }

  physx::PxTriangleMeshDesc groundDesc;
  groundDesc.setToDefault();

  groundDesc.points.count = levelVertices.size();
  groundDesc.points.stride = sizeof(physx::PxVec3);
  groundDesc.points.data = levelVertices.data();

  groundDesc.triangles.count = levelIndices.size() / 3;
  groundDesc.triangles.stride = sizeof(physx::PxU32) * 3;
  groundDesc.triangles.data = levelIndices.data();

  physx::PxDefaultMemoryOutputStream writeBuffer;
  physx::PxTriangleMeshCookingResult::Enum result;

  bool status = gCooking->cookTriangleMesh(groundDesc, writeBuffer, &result);
  if (!status) // result == physx::PxTriangleMeshCookingResult::Enum::eFAILURE)
  {
    std::cerr << "Mesh cooking failed...  who let physx cook?" << std::endl;
  }
  physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());

  return gPhysics->createTriangleMesh(readBuffer);
}

void MeshCollider::initLevelRigidBody(physx::PxTriangleMesh *levelMesh)
{

  physx::PxMaterial *levelMaterial = m_physicsSystem->m_Physics->createMaterial(0.5f, 0.5f, 0.2f);
  auto gPhysics = m_physicsSystem->m_Physics;
  auto material = levelMaterial;
  auto scene = m_physicsSystem->m_Scene;

  physx::PxTriangleMeshGeometry levelGeo = physx::PxTriangleMeshGeometry(levelMesh, physx::PxMeshScale(1));
  m_levelShape = gPhysics->createShape(levelGeo, *material, true);

  physx::PxTransform levelTransform(physx::PxVec3(0, 0, 0), physx::PxQuat(physx::PxIdentity));
  m_level = gPhysics->createRigidStatic(levelTransform);

  m_level->attachShape(*m_levelShape);
  scene->addActor(*m_level);
}

void MeshCollider::release()
{
  m_levelShape->release();
  m_level->release();
}

void MeshCollider::initLevelRigidBody(physx::PxTriangleMesh *levelMesh, physx::PxMaterial *material)
{

  auto gPhysics = m_physicsSystem->m_Physics;
  auto scene = m_physicsSystem->m_Scene;

  physx::PxTriangleMeshGeometry levelGeo = physx::PxTriangleMeshGeometry(levelMesh, physx::PxMeshScale(1));
  m_levelShape = gPhysics->createShape(levelGeo, *material, true);

  physx::PxTransform levelTransform(physx::PxVec3(0, 0, 0), physx::PxQuat(physx::PxIdentity));
  physx::PxRigidStatic *m_level = gPhysics->createRigidStatic(levelTransform);

  m_level->attachShape(*m_levelShape);
  scene->addActor(*m_level);
}
