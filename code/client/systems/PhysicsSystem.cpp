#include "PxPhysicsAPI.h"
#include "PhysicsSystem.h"
#include "utils/Time.h"
#include "../Car.h"
#include "../AICar.h"

namespace physics
{

    void PhysicsSystem::Initialize()
    {
		initPhysX();
		initGroundPlane();
		initMaterialFrictionTable();
        initCooking(); 
		// if (!initVehicles())
		// 	return false;
		// return true;
    }


	void PhysicsSystem::Update(ecs::Scene& scene, float deltaTime) {
        // get all physics components that need updates & process them in here
    
        // first, update any player controlled vehicles
        for (auto entityGuid : ecs::EntitiesInScene<Car>(scene))
        {
            // get the car, and update it.
            Car& car = scene.GetComponent<Car>(entityGuid);
            car.Update(entityGuid,scene,deltaTime);
        }

        // then, update all ai controlled vehicles
        for (auto entityGuid : ecs::EntitiesInScene<AICar>(scene))
        {
            AICar& aicar = scene.GetComponent<AICar>(entityGuid);
            aicar.Update(entityGuid,scene,deltaTime);
        }

        // Forward integrate the phsyx scene by a single timestep.
        m_Scene->simulate(deltaTime);
        m_Scene->fetchResults(true);


    }

    void PhysicsSystem::Update(ecs::Scene& scene, Timestep timestep)
    {
        Update(scene, timestep.getSeconds());
    }


    void PhysicsSystem::Cleanup()
    {
        cleanupGroundPlane();
        cleanupPhysX();
    }


    void PhysicsSystem::initPhysX()
    {
        m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback);
        m_Pvd = PxCreatePvd(*m_Foundation);
        PxPvdTransport *transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
        m_Pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
        m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(), true, m_Pvd);

        PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
        sceneDesc.gravity = m_Gravity;

        PxU32 numWorkers = 1;
        m_Dispatcher = PxDefaultCpuDispatcherCreate(numWorkers);
        sceneDesc.cpuDispatcher = m_Dispatcher;
        sceneDesc.filterShader = VehicleFilterShader;

        m_Scene = m_Physics->createScene(sceneDesc);
        PxPvdSceneClient *pvdClient = m_Scene->getScenePvdClient();
        if (pvdClient)
        {
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
        }
        // First value is static friction (how hard it is to move an object from rest)
        // Second value is dynamic friction (how much resistance does it provide to an object moving accross the surface)
        // Third value is Restitution (how much force is returned upon collision, in other words how bouncy is the material)
        m_Material = m_Physics->createMaterial(0.5f, 0.5f, 0.5f);
        //m_Material = m_Physics->createMaterial(0.5f, 1.0f, 0.1f);

        PxInitVehicleExtension(*m_Foundation);

    }

    void PhysicsSystem::cleanupPhysX()
    {
        PxCloseVehicleExtension();

        PX_RELEASE(m_Material);
        PX_RELEASE(m_Scene);
        PX_RELEASE(m_Dispatcher);
        PX_RELEASE(m_Physics);
        if (m_Pvd)
        {
            PxPvdTransport *transport = m_Pvd->getTransport();
            m_Pvd->release();
            transport->release();
        }
        PX_RELEASE(m_Foundation);
    }



    void PhysicsSystem::initMaterialFrictionTable()
    {
        // Each physx material can be mapped to a tire friction value on a per tire basis.
        // If a material is encountered that is not mapped to a friction value, the friction value used is the specified default value.
        // In this snippet there is only a single material so there can only be a single mapping between material and friction.
        // In this snippet the same mapping is used by all tires.
        m_PhysXMaterialFrictions[0].friction = 1.0f;
        m_PhysXMaterialFrictions[0].material = m_Material;
        m_PhysXDefaultMaterialFriction = 1.0f;
        m_NbPhysXMaterialFrictions = 1;
    }

    void PhysicsSystem::initGroundPlane()
    {
        // Can't set PxPlane to all zero's 
        gGroundPlane = PxCreatePlane(*m_Physics, PxPlane(0, 1, 0, 0), *m_Material);
        //std::cout << "Material: " << gMaterial->getStaticFriction() << std::endl;
        for (PxU32 i = 0; i < gGroundPlane->getNbShapes(); i++)
        {
            PxShape *shape = NULL;
            gGroundPlane->getShapes(&shape, 1, i);
            shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
            shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
            shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
        }
        m_Scene->addActor(*gGroundPlane);
    }


    void PhysicsSystem::initCooking()
    {
        // Level
        m_CookingParams = &physx::PxCookingParams(m_Physics->getTolerancesScale());
        m_Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation, *m_CookingParams);
        if (!m_Cooking)
        {
            std::cerr << "PxCreateCooking failed!" << std::endl;
        }

        m_CookingParams->meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
        m_CookingParams->meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
    }

    // TODO(dylan): cleanup functions for level collider

    void PhysicsSystem::cleanupGroundPlane()
    {
        gGroundPlane->release();
    }

}