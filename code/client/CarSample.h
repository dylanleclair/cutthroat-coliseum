// #include <ctype.h>
// #include <iostream>

// #include "PxPhysicsAPI.h"
// #include "vehicle2/PxVehicleAPI.h"
// #include "physx/snippetvehicle2common/enginedrivetrain/EngineDrivetrain.h"
// #include "physx/snippetvehicle2common/serialization/BaseSerialization.h"
// #include "physx/snippetvehicle2common/serialization/EngineDrivetrainSerialization.h"
// #include "physx/snippetvehicle2common/SnippetVehicleHelpers.h"

// #include "physx/snippetcommon/SnippetPVD.h"

// #include "Time.h"
// #include "SDL.h"
// #include <limits>

// using namespace physx;
// using namespace physx::vehicle2;
// using namespace snippetvehicle2;

// // PhysX management class instances.

// extern PxDefaultAllocator gAllocator;
// extern PxDefaultErrorCallback gErrorCallback;
// extern PxFoundation* gFoundation;
// extern PxPhysics* gPhysics;
// extern PxDefaultCpuDispatcher* gDispatcher;
// extern PxScene* gScene;
// extern PxMaterial* gMaterial;
// extern PxPvd* gPvd;


// // The path to the vehicle json files to be loaded.
// extern const char* gVehicleDataPath;

// // The vehicle with engine drivetrain
// extern EngineDriveVehicle gVehicle;

// // Vehicle simulation needs a simulation context
// // to store global parameters of the simulation such as
// // gravitational acceleration.
// extern PxVehiclePhysXSimulationContext gVehicleSimulationContext;

// // Gravitational acceleration
// extern const PxVec3 gGravity;

// // The mapping between PxMaterial and friction.
// extern PxVehiclePhysXMaterialFriction gPhysXMaterialFrictions[16];
// extern PxU32 gNbPhysXMaterialFrictions;
// extern PxReal gPhysXDefaultMaterialFriction;

// // Give the vehicle a name so it can be identified in PVD.
// extern const char gVehicleName[];

// // Commands are issued to the vehicle in a pre-choreographed sequence.
// struct Command
// {
//     PxF32 brake;
//     PxF32 throttle;
//     PxF32 steer;
//     PxU32 gear;
//     // PxF32 duration;
// };
// extern const PxU32 gTargetGearCommand;
// extern Command gCommands[5];
// extern const PxU32 gNbCommands;
// extern PxReal gCommandTime; // Time spent on current command
// extern PxU32 gCommandProgress; // The id of the current command.

// // A ground plane to drive on.
// extern PxRigidStatic* gGroundPlane;

// void initPhysX();
// void cleanupPhysX();
// void initGroundPlane();
// void cleanupGroundPlane();
// void initMaterialFrictionTable();
// bool initVehicles();
// void cleanupVehicles();
// bool initPhysics();
// void cleanupPhysics();
// PxRigidBody* getVehicleRigidBody();



// // HACK(beau): make these visible to tuning imgui panel
// extern float carThrottle;
// extern float carBrake;
// extern float carAxis;
// extern float carAxisScale;

// // this operates on the vehicle
// extern void stepPhysics(SDL_GameController* controller, Timestep timestep);
// // inits car sample stuff ig
// extern int carSampleInit();

