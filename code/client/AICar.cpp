
#include "AICar.h"
#include "systems/ai.h"
#include "systems/pathfinding.h"
#include "systems/GraphicsSystem.h"
#include "utils/PxConversionUtils.h"
#include "glm/glm.hpp"

glm::vec3 roundPositionToGraph(glm::vec3 pos)
{
    return glm::vec3(floor(pos.x), 0.f, floor(pos.z));
}

float euclideanBasic(glm::vec3 a, glm::vec3 b)
{
    float deltaX = b.x - a.x;
    float deltaZ = b.z - a.z;

    float dist = pow(deltaX, 2) + pow(deltaZ, 2);
    return sqrt(dist);
}

void AICar::Update(Guid carGuid,ecs::Scene& scene, float deltaTime) 
{


  Command command = {0.f, 0.f, 0.f, m_TargetGearCommand};

    // need to get access to the pathfinding system....

    // so the job of the pathfinding system is to:
    //  - calculate the path to the opponent
    //  - after that, the physics system will update the car (by calling this function)
    //      - this should make the car follow the path

    if (scene.HasComponent<PathfindingComponent>(carGuid))
    {
        // the car has a pathfinder !!
        // must move according to path. 

        // begin by computing the path using the navmesh!

        // psuedocode for desired: 
        // NavMesh.search(startPosition, endPosition);
        // the navmesh will:
        //  - find the appropriate start node from start position
        //  - find the appropriate end node from finish position
        //  - find the path of nodes that will lead to destination
        //  - compute a spline path to follow ??              

        // foil out the nodes !
        CPU_Geometry pathGeom;

        PathfindingComponent& p = scene.GetComponent<PathfindingComponent>(carGuid);
        // get the pathfinding component
        TransformComponent& position = scene.GetComponent<TransformComponent>(carGuid);
        // find the target
        Guid targetEntity = p.targetEntity;

        if (scene.HasComponent<TransformComponent>(targetEntity))
        {
            TransformComponent targetPosition = scene.GetComponent<TransformComponent>(targetEntity);
            std::vector<glm::vec3> path = pathfinding::AStar<glm::vec3>(roundPositionToGraph(position.getTranslation()), roundPositionToGraph(targetPosition.getTranslation()), euclideanBasic, AISystem::generateNearby);

            for (auto& pos : path)
            {
                pathGeom.verts.push_back(glm::vec3{ pos });
            }
            // TODO: attempt to smooth the path with chaikin
        
        // hook into a renderer so we can see if it's correct...

        // goal for now: have AI car simply follow the player-driven car
        // 
        // find rotation matrix of car
        PxTransform carPose = this->m_Vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose();

        // find the direction vector of the vehicle
        glm::quat vehicleQuat = PxtoGLM(carPose.q);
        glm::mat4 vehicleRotM = glm::toMat4(vehicleQuat);
        glm::vec3 heading = glm::vec3{vehicleRotM * glm::vec4{0.f, 0.f, -1.f, 1.f}};
        // need to somehow find the rotation of the vehicle?
        
        // we have the forward vector of vehicle now
        // target position
        PxVec3 targetDir = GLMtoPx(targetPosition.getTranslation()) - carPose.p;
        
        if (targetDir.magnitude() < 8.f)
        {
            command.throttle = 0.f;
        } else {
            command.throttle = 1.f;
        }
        
        // targetDir = glm::normalize(targetDir);
        targetDir.normalize();
        // float angleBetween = glm::dot(targetDir,heading);

        float angleBetween = targetDir.dot(GLMtoPx(heading));

        std::cout << "angle between heading and target dir: " << abs(angleBetween) << "\n";

        // if almost parallel, don't worry about steering
        if (abs(angleBetween) > 0.95f)
        {
            command.steer = 0.0f;
        } else {
            PxVec3 cross = GLMtoPx(heading).cross(targetDir);
            if (cross.y < 0)
            {
             command.steer = 1.0f;
            } else {
             command.steer = -1.0f;
            }
        }

    

        }


        // draw this later -> have to use global renderer
        // if (scene.HasComponent<RenderLine>(carGuid))
        // {
        //     RenderLine& line = scene.GetComponent<RenderLine>(carGuid);
        //     line.setGeometry(pathGeom);
        // }    




    }

  float delta_seconds = deltaTime;
  assert(delta_seconds > 0.f && delta_seconds < 0.2000001f);
  // Apply the brake, throttle and steer to the command state of the vehicle.
  // const Command &command = gCommands[gCommandProgress];

  // command.duration = timestep;

  // Throttle to 2.f will cause weird behaviour
//   if (SDL_GameControllerGetButton(ControllerInput::controller, SDL_CONTROLLER_BUTTON_A) || w_key)
//   {
//       command.throttle = carThrottle;
//       // goto end; // so we don't attempt to throttle and break
//   }
//   else if (SDL_GameControllerGetButton(ControllerInput::controller, SDL_CONTROLLER_BUTTON_B) || s_key)
//   {
//       command.brake = carBrake;
//       // goto end;????
//   }
  // end:

  // Normalize controller axis
  // BUG: max positive is 1 less in magnitude than max min meaning full negative will be slightly above 1
//   carAxis = (float)-SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_LEFTX) / SHRT_MAX;
//   // std::cout << axis << std::endl;
//   if (a_key)
//   {
//       command.steer = 1.f;
//   }
//   else if (d_key)
//   {
//       command.steer = -1.f;
//   }
//   else
//   {
//       command.steer = carAxis * carAxisScale;
//   }  


  m_Vehicle.mCommandState.brakes[0] = command.brake;
  m_Vehicle.mCommandState.nbBrakes = 1;
  m_Vehicle.mCommandState.throttle = command.throttle;
  m_Vehicle.mCommandState.steer = command.steer;
  m_Vehicle.mTransmissionCommandState.targetGear = command.gear;

  // Forward integrate the vehicle by a single timestep.
  // Apply substepping at low forward speed to improve simulation fidelity.
  const PxVec3 linVel = m_Vehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity();
  const PxVec3 forwardDir = m_Vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.getBasisVector2();
  const PxReal forwardSpeed = linVel.dot(forwardDir);
  const PxU8 nbSubsteps = (forwardSpeed < 5.0f ? 3 : 1);
  m_Vehicle.mComponentSequence.setSubsteps(m_Vehicle.mComponentSequenceSubstepGroupHandle, nbSubsteps);
  m_Vehicle.step(delta_seconds, m_VehicleSimulationContext);

  // Forward integrate the phsyx scene by a single timestep.
  physicsSystem->m_Scene->simulate(delta_seconds);
  physicsSystem->m_Scene->fetchResults(true);
}

