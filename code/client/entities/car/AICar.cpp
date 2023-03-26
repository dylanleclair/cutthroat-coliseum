
#include "AICar.h"
#include "core/pathfinding.h"
#include "../../systems/GraphicsSystem.h"
#include "../../utils/PxConversionUtils.h"
#include "glm/glm.hpp"
#include "../physics/LevelCollider.h"
#include <iostream>

void AICar::Initialize(NavPath* pathToFollow)
{
    m_navPath = pathToFollow;
}

/** 
 * Will cast a ray from the origin and direction to a maximum dist, returning true
 * iff the shape it collides with is the provided shape. 
*/
bool castRay(PxScene* scene, PxVec3 origin, PxVec3 dir, float dist, physx::PxShape* target_shape)
{
    PxRaycastBuffer hit;

    bool status = scene->raycast(origin, dir, dist, hit);
    if (status)
    {
        if (hit.block.shape == target_shape)
        {
            return true;
        }

    }
    return false;
}

Command AICar::pathfind(glm::vec3 currentPosition, ecs::Scene& scene, float deltaTime)
{

    assert(m_navPath != nullptr);

    Command command = {0.f, 0.f, 0.f, m_TargetGearCommand};

    bool didLap{false};

    glm::vec3 targetPos = m_navPath->getNextPoint(currentPosition,didLap);

    // find rotation matrix of car
    PxTransform carPose = this->m_Vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose();

    glm::vec3 glm_carPose = PxtoGLM(carPose.p);

    glm::vec4 carLocalForward{0.f, 0.f, -1.f, 1.f};

    // find the direction vector of the vehicle
    glm::quat vehicleQuat = PxtoGLM(carPose.q);
    glm::mat4 vehicleRotM = glm::toMat4(vehicleQuat);
    glm::vec3 headingDir = glm::vec3{vehicleRotM * glm::vec4{0.f, 0.f, 1.f, 1.f}};
    
    // const int num_rays{15};
    std::vector<glm::vec3> steering_rays;

    // for (int i = 0; i < num_rays; i++)
    // {
    //     // rotate pi / numrays around y axis
    //     float rot_angle = M_PI / num_rays;
    //     auto M = glm::rotate(glm::mat4{1.f}, rot_angle, headingDir);
    //     steering_rays.push_back(glm::vec3{vehicleRotM * M * carLocalForward});
    // }

    // need to generate a whole array of raycasts to determine which way to turn?
    // or compute direction vector on the track // via waypoints!

    // cast a ray for each steering_ray

    glm::vec3 up{0.f,1.f,0.f};

    float rot_angle = M_PI / 5;
    auto M = glm::rotate(glm::mat4{1.f}, -rot_angle, up);
    steering_rays.push_back(glm::vec3{vehicleRotM * M * carLocalForward});

    M = glm::rotate(glm::mat4{1.f}, rot_angle, up);
    steering_rays.push_back(glm::vec3{vehicleRotM * M * carLocalForward});

    // look into behaviour tree
    // - if by wall, steer away
    // - if stuck, reverse out

    LevelCollider* level_c;
    ObstacleCollider* obs_c;

    for (Guid entity : ecs::EntitiesInScene<LevelCollider>(scene))
    {
        // get the level collider
        LevelCollider& lc = scene.GetComponent<LevelCollider>(entity);
        level_c = &lc;
        break;
    }

    // for (Guid entity : ecs::EntitiesInScene<MeshCollider>(scene))
    // {
    //     // get the level collider
    //     ObstacleCollider& oc = scene.GetComponent<ObstacleCollider>(entity);
    //     obs_c = &oc;
    //     break;
    // }


    bool forced_turn_left{false};
    bool forced_turn_right{false};
    bool hitting_wall{false};
    bool obstacle_ahead{false};

    // split into left and right
    bool hit = castRay(physicsSystem->m_Scene, carPose.p, GLMtoPx(steering_rays[0]), 10.f, level_c->getShape());
    if (hit)
    {
        // ray hit forward left
        forced_turn_right = true;
    }

    hit = castRay(physicsSystem->m_Scene, carPose.p, GLMtoPx(steering_rays[1]), 10.f, level_c->getShape());
    if (hit)
    {
        // ray hit forward right
        forced_turn_left = true;
    }

    hitting_wall = castRay(physicsSystem->m_Scene, carPose.p, GLMtoPx(headingDir), 7.f, level_c->getShape());
    if (hitting_wall)
    {
        m_stuckTimer += deltaTime;
    }


    // obstacle_ahead = castRay(physicsSystem->m_Scene, carPose.p, GLMtoPx(headingDir), 7.f, obs_c->getShape());
    // if (obstacle_ahead)
    // {

    // }


    // PxVec3 targetDir = ( collided_rays.size() == 0 ) ? GLMtoPx(m_navPath->getDirectionVector(PxtoGLM(carPose.p))) : GLMtoPx(compute_target_dir(collided_rays));
    // PxVec3 targetDir = GLMtoPx(m_navPath->getDirectionVector(PxtoGLM(carPose.p)));

    // target direction vector
    PxVec3 targetDir = GLMtoPx(targetPos) - carPose.p;
    // only drive to the target if it's far enough away (for now)

    // need to scale down throttle based on the angle we're trying to turn
    // this will help the AI turn!

    // get the track direction, slow and turn until we are parallel with track dir!

    command.throttle = 1.f;

    targetDir.normalize();

    // radians!!

    float angleBetween = targetDir.dot(GLMtoPx(headingDir));

    float actualAngle = acos(angleBetween);

    // if almost parallel, don't worry about steering
    if (abs(actualAngle) < 0.08f)
    {
        command.steer = 0.0f;
    } else {
        
        // want to scale steering more gently based on the actual angle

        // compute direction to turn
        PxVec3 cross = GLMtoPx(headingDir).cross(targetDir);
        float turn_dir = (cross.y < 0) ? -1.f : 1.f;

        // float normalized "turning" angle


        if (forced_turn_left || forced_turn_right)
        {
            command.throttle = 0.5f;
            
            // scale steering based on distance to wall?

            if (forced_turn_left)
            {
                command.steer = 0.45f;
            } else {
                command.steer = -0.45f;
            }
        } else {
            command.throttle = 1.f;
            // normal turning logic
            if (actualAngle < M_PI_4) // if the turning angle is more gentle
            {
                // if close to steering the right direction, make steering less powerful
                command.steer = turn_dir * (actualAngle / M_PI_4);
            } else {
                command.steer = turn_dir;
            }
        }

    }

    if (m_stuckTimer > 0.8f && hitting_wall)
    {
        // reverse
        m_TargetGearCommand = 0; // put in reverse
        command.throttle = 0.5f;

        // calulate the way they want to steer to be in line with the track!
        {
            auto targetDir = GLMtoPx(m_navPath->getDirectionVector(glm_carPose));

            PxVec3 cross = GLMtoPx(headingDir).cross(targetDir);
            float turn_dir = (cross.y < 0) ? -1.f : 1.f;

            command.steer = -turn_dir; // flip the turn direction in reverse!
        }        

    }

    if (!hitting_wall)
    {
        // if not hitting wall, reset stuck timer
        m_stuckTimer = 0;
        this->m_TargetGearCommand = 255;

    }

    checkFlipped(carPose);

    return command;
}

void AICar::Update(Guid carGuid,ecs::Scene& scene, float deltaTime) 
{

  Command command = {0.f, 0.f, 0.f, m_TargetGearCommand};

    if (scene.HasComponent<TransformComponent>(carGuid))
    {
        TransformComponent& tComponent = scene.GetComponent<TransformComponent>(carGuid);
        auto currentPos = tComponent.getTranslation();
        command = pathfind(currentPos, scene, deltaTime);
    }   

  float delta_seconds = deltaTime;
  assert(delta_seconds > 0.f && delta_seconds < 0.2000001f);

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








// UNUSED, keeping for later
glm::vec3 compute_target_dir(std::vector<glm::vec3>& collidedRays)
{
    // compute the average of the collided rays
    glm::vec3 average = {0.f,0.f,0.f};
    for (int i = 0; i < collidedRays.size(); i++)
    {
        average += collidedRays[i];
    }
    average /= collidedRays.size();

    // the average vector now completed. flip x and z axis.
    average.x *= -1;
    average.z *= -1;
    // maybe y?

    return average;
}
