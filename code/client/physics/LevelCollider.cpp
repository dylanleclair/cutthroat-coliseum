#include "PxPhysicsAPI.h"
#include "LevelCollider.h"
#include <vector>
#include <iostream>
#include "systems/GraphicsSystem.h"
#include <string>


    LevelCollider::LevelCollider(std::string modelName, PhysicsSystem& physics) : m_physicsSystem(physics)
    {
        CPU_Geometry levelGeom{};
        GraphicsSystem::importOBJ(levelGeom, modelName);
        // convert the verts / indices
        for (auto vert : levelGeom.verts)
        {
            levelVertices.push_back(GLMtoPx(vert));
        }

        for (auto& index : levelGeom.indicies)
        {
            levelIndices.push_back(index);
        }

    }

    LevelCollider::LevelCollider(CPU_Geometry& levelGeom, PhysicsSystem& physics) : m_physicsSystem(physics)
    {
        // passes the geom

        for (auto vert : levelGeom.verts)
        {
            levelVertices.push_back(GLMtoPx(vert));
        }

        for (auto& index : levelGeom.indicies)
        {
            levelIndices.push_back(index);
        }

    }


physx::PxTriangleMesh * LevelCollider::cookLevel(glm::mat4 transform) 
	{
    auto gCooking = m_physicsSystem.m_Cooking;
    auto gPhysics = m_physicsSystem.m_Physics;


    for (auto& vert : levelVertices)
    {
        vert = GLMtoPx(glm::vec4{PxtoGLM(vert),1.f} * transform); 
    }

    physx::PxTriangleMeshDesc groundDesc;
	groundDesc.setToDefault();

	groundDesc.points.count = levelVertices.size();
	groundDesc.points.stride = sizeof(physx::PxVec3);
	groundDesc.points.data = levelVertices.data();

	groundDesc.triangles.count = levelIndices.size();
	groundDesc.triangles.stride = sizeof(physx::PxU32) * 3;
	groundDesc.triangles.data = levelIndices.data();

	physx::PxDefaultMemoryOutputStream writeBuffer;
	physx::PxTriangleMeshCookingResult::Enum result;

    for (int i = 0; i < 10; i++) {
        gCooking->cookTriangleMesh(groundDesc, writeBuffer, &result);
        if (result == physx::PxTriangleMeshCookingResult::Enum::eFAILURE)
        {
            std::cerr << "level cooking failed...  who let physx cook?" << std::endl;
            continue;
        }
    }
	physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());

    return gPhysics->createTriangleMesh(readBuffer);

    }

    void LevelCollider::initLevelRigidBody(physx::PxTriangleMesh* levelMesh)
    {
        auto gPhysics = m_physicsSystem.m_Physics;
        auto material = m_physicsSystem.m_Material;
        auto scene = m_physicsSystem.m_Scene;

        physx::PxTriangleMeshGeometry levelGeo = physx::PxTriangleMeshGeometry(levelMesh, physx::PxMeshScale(1));
	    physx::PxShape* levelShape = gPhysics->createShape(levelGeo, *material, true);

        physx::PxTransform levelTransform(physx::PxVec3(0,0,0), physx::PxQuat(physx::PxIdentity));
        physx::PxRigidStatic* level = gPhysics->createRigidStatic(levelTransform);

        level->attachShape(*levelShape);
        scene->addActor(*level);
    }
    

