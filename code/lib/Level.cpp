#include "PxPhysicsAPI.h"
#include "Level.h"
#include <vector>
#include <iostream>
#include "systems/GraphicsSystem.h"
#include <string>

void initLevelCooking(
    physx::PxTolerancesScale *gTolerances,
    physx::PxCookingParams *gLevelCookingParams,
    physx::PxCooking *gCooking,
    physx::PxTriangleMesh *gLevelMesh,
    physx::PxFoundation *gFoundation)
{
    // Level
    gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, *gLevelCookingParams);
    if (!gCooking)
    {
        std::cerr << "PxCreateCooking failed!" << std::endl;
    }
    std::vector<physx::PxVec3> levelVertices = std::vector<physx::PxVec3>();
    std::vector<physx::PxU32> levelIndices = std::vector<physx::PxU32>();

    gLevelCookingParams->meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
    gLevelCookingParams->meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
}

struct Level
{

    Level(std::string modelName)
    {
        CPU_Geometry levelGeom;
        GraphicsSystem::importOBJ(levelGeom, modelName);
        // convert the verts / indices
        
        // can probably save memory if we can just directly use CPU_Geometry as is
        for (auto vert : levelGeom.verts)
        {
            levelVertices.push_back(GLMtoPx(vert));
        }

        for (auto& index : levelGeom.indicies)
        {
            levelIndices.push_back(index);
        }

    }

    Level(CPU_Geometry& levelGeom)
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


physx::PxTriangleMesh * cookLevel(std::vector<physx::PxVec3> levelVertices, std::vector<physx::PxU32> levelIndices, physx::PxPhysics* gPhysics, physx::PxCooking *gCooking) 
	{
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

	gCooking->cookTriangleMesh(groundDesc, writeBuffer,&result);
	if (result == physx::PxTriangleMeshCookingResult::Enum::eFAILURE)
	{
		std::cerr << "level cooking failed...  who let physx cook?" << std::endl;
	}
	physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());

    return gPhysics->createTriangleMesh(readBuffer);

    }

    void initLevelRigidBody(physx::PxPhysics* gPhysics, physx::PxScene* scene, physx::PxMaterial* material, physx::PxTriangleMesh* levelMesh)
    {
        physx::PxTriangleMeshGeometry levelGeo = physx::PxTriangleMeshGeometry(levelMesh, physx::PxMeshScale(1));
	    physx::PxShape* levelShape = gPhysics->createShape(levelGeo, *material, true);

        physx::PxTransform levelTransform(physx::PxVec3(0,0,0), physx::PxQuat(physx::PxIdentity));
        physx::PxRigidStatic* level = gPhysics->createRigidStatic(levelTransform);

        level->attachShape(*levelShape);
        scene->addActor(*level);

    }
    

private:
    std::vector<physx::PxVec3> levelVertices = std::vector<physx::PxVec3>();
    std::vector<physx::PxU32> levelIndices = std::vector<physx::PxU32>();
};