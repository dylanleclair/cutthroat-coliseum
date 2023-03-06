
#include "AICar.h"
#include "systems/ai.h"
#include "systems/pathfinding.h"
#include "systems/GraphicsSystem.h"
#include "utils/PxConversionUtils.h"
#include "glm/glm.hpp"


NavPath generateCirclePath(int radius) {

    // generate a circle
    std::vector<glm::vec3> circle;

    float step{0.4f};
    for (float i =0; i < 2 * 3.14; i += step)
    {
        float x = std::cos(i);
        float z = std::sin(i);
        circle.push_back({radius * x, 0.f, radius * z});
    }

    return NavPath{circle};

}


void AICar::Initialize(NavPath* pathToFollow)
{
    m_navPath = pathToFollow;
}

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

Command AICar::pathfind(glm::vec3 currentPosition)
{

    assert(m_navPath != nullptr);

    Command command = {0.f, 0.f, 0.f, m_TargetGearCommand};

    bool didLap{false};

    glm::vec3 targetPos = m_navPath->getNextPoint(currentPosition,didLap);

    if (didLap)
    {
        m_lapCount++;
        std::cout << "AI completed a lap!" << std::endl;
        std::cout << "AI starting lap: " << m_lapCount << std::endl;
    }

    if (m_lapCount == 3)
    {
        std::cout << "AI wins!" << std::endl;
    }

    std::vector<glm::vec3> path = pathfinding::AStar<glm::vec3>(roundPositionToGraph(currentPosition), roundPositionToGraph(targetPos), euclideanBasic, AISystem::generateNearby);

    // find rotation matrix of car
    PxTransform carPose = this->m_Vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose();

    // find the direction vector of the vehicle
    glm::quat vehicleQuat = PxtoGLM(carPose.q);
    glm::mat4 vehicleRotM = glm::toMat4(vehicleQuat);
    glm::vec3 headingDir = glm::vec3{vehicleRotM * glm::vec4{0.f, 0.f, -1.f, 1.f}};
    
    // target direction vector
    PxVec3 targetDir = GLMtoPx(targetPos) - carPose.p;
    
    // only drive to the target if it's far enough away (for now)

    command.throttle = .7f;


    targetDir.normalize();

    float angleBetween = targetDir.dot(GLMtoPx(headingDir));

    // if almost parallel, don't worry about steering
    if (abs(angleBetween) > 0.95f)
    {
        command.steer = 0.0f;
    } else {
        PxVec3 cross = GLMtoPx(headingDir).cross(targetDir);
        if (cross.y < 0)
        {
            command.steer = 1.0f;
        } else {
            command.steer = -1.0f;
        }
    }

    checkFlipped(carPose);


    return command;
}

Command AICar::pathfind(glm::vec3 currentPosition, ecs::Scene& scene, Guid targetEntity)
{
    assert(m_navPath != nullptr);

    Command command = {0.f, 0.f, 0.f, m_TargetGearCommand};

    if (scene.HasComponent<PathfindingComponent>(targetEntity))
    {
        // the car has a pathfinder !!
        // must move according to path. 

        // foil out the nodes !
        CPU_Geometry pathGeom;

        PathfindingComponent& p = scene.GetComponent<PathfindingComponent>(targetEntity);
        // get the pathfinding component
        TransformComponent& position = scene.GetComponent<TransformComponent>(targetEntity);
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
            // -> this only makes sense for large graphs so chill for now
        
        // hook into a renderer so we can see if it's correct...

        // goal for now: have AI car simply follow the player-driven car
        // 
        // find rotation matrix of car
        PxTransform carPose = this->m_Vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose();

        // find the direction vector of the vehicle
        glm::quat vehicleQuat = PxtoGLM(carPose.q);
        glm::mat4 vehicleRotM = glm::toMat4(vehicleQuat);
        glm::vec3 headingDir = glm::vec3{vehicleRotM * glm::vec4{0.f, 0.f, -1.f, 1.f}};
        
        // target direction vector
        PxVec3 targetDir = GLMtoPx(targetPosition.getTranslation()) - carPose.p;
        
        // only drive to the target if it's far enough away (for now)
        if (targetDir.magnitude() < 8.f)
        {
            command.throttle = 0.f;
        } else {
            command.throttle = 1.f;
        }

        targetDir.normalize();

        float angleBetween = targetDir.dot(GLMtoPx(headingDir));

        // if almost parallel, don't worry about steering
        if (abs(angleBetween) > 0.95f)
        {
            command.steer = 0.0f;
        } else {
            PxVec3 cross = GLMtoPx(headingDir).cross(targetDir);
            if (cross.y < 0)
            {
             command.steer = 1.0f;
            } else {
             command.steer = -1.0f;
            }
        }

        }

    }

    return command;
}


void AICar::Update(Guid carGuid,ecs::Scene& scene, float deltaTime) 
{


  Command command = {0.f, 0.f, 0.f, m_TargetGearCommand};

    // need to get access to the pathfinding system....

    // so the job of the pathfinding system is to:
    //  - calculate the path to the opponent
    //  - after that, the physics system will update the car (by calling this function)
    //      - this should make the car follow the path

    if (scene.HasComponent<TransformComponent>(carGuid))
    {
        TransformComponent& tComponent = scene.GetComponent<TransformComponent>(carGuid);
        auto currentPos = tComponent.getTranslation();
        command = pathfind(currentPos);
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


}

