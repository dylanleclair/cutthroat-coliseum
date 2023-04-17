#include "Car.h"
#include "../../Input.h"
#include "glm/glm.hpp"
#include <glm/gtx/projection.hpp>
#include "../../utils/PxConversionUtils.h"

#include "../physics/LevelCollider.h"
#include "../../systems/RaceSystem.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

physx::PxShape* castRayCheckShape(PxScene* scene, PxVec3 origin, PxVec3 dir, float dist)
{
    PxRaycastBuffer hit;

    bool status = scene->raycast(origin, dir, dist, hit);
    if (status)
    {
        return hit.block.shape;
    }
    return nullptr;
}

const char* gVehicleDataPath = "vehicledata";

// // HACK(beau): make these visible to tuning imgui panel
float carThrottle = 1.f;
float carBrake = 1.f;
float carAxis = 0.f;
float carAxisScale = 1.f;

float controller_throttle = 0.f;
float controller_brake = 0.f;

int time_elapsed = 0;
bool has_jumped = false;
bool previous_b_press = false;

PxTransform c_mass_init_v;
PxReal angular_damp_init_v;


glm::vec3 Car::getTrackNormal()
{
    return m_track->normal(getPosition());
}

/** chat gpt + dylan wizardy */
void Car::keepRigidbodyUpright(PxRigidBody* rigidbody)
{

        if (!m_grounded)
        // if ( m_Vehicle.mBaseState.tireSlipStates->slips[0] > 150.f)
        {
            // correct the vehicles orientation

            m_Vehicle.mPhysXState.physxActor.rigidBody->setAngularDamping(10.f);
            PxTransform transform = rigidbody->getGlobalPose();

            PxVec3 upVector = transform.q.getBasisVector1(); // get the up vector from the rigidbody's orientation

            upVector.normalize();

            // make the desired up vector the normal of the track (closest to the car)
            PxVec3 desiredUpVector =GLMtoPx(getTrackNormal()); 

            PxVec3 axis = upVector.cross(desiredUpVector);
            PxReal angle = acos(upVector.dot(desiredUpVector));

            // // Apply a torque to the rigidbody to rotate it towards the desired orientation
            PxVec3 torque = axis * angle * rigidbody->getMass() * STRENGTH_UP_CORRECTION; // adjust the torque magnitude as needed
            rigidbody->addTorque(torque, PxForceMode::eFORCE);

            // now do it for the other axis

            // forward dir is z
            // so now we need to straighten X out too (if we can)

            glm::vec3 forwardCar = getForwardDir();
            PxVec3 xDir = transform.q.getBasisVector0();
            PxVec3 binormalCar = desiredUpVector.cross(GLMtoPx(forwardCar));

            axis = xDir.cross(binormalCar);
            angle = acos(xDir.dot(binormalCar));

            // Apply a torque to the rigidbody to rotate it towards the desired orientation
            torque = axis * angle * rigidbody->getMass() * STRENGTH_UP_CORRECTION; // adjust the torque magnitude as needed
            rigidbody->addTorque(torque, PxForceMode::eFORCE);
            
            // the combined effect of these two changes it to lock the yaw and roll of the car. 

        } else {
            m_Vehicle.mPhysXState.physxActor.rigidBody->setAngularDamping(angular_damp_init_v);
        }


}



void Car::Initialize(DriverType type, PxTransform initialPose, physics::PhysicsSystem* ps, Curve* track, NavPath* pathToFollow)
{

  if (!ps)
  {
    std::cerr << "Physics system (null) must be initialized to initialize a vehicle.\n";
    return;
  }

  if (!pathToFollow)
  {
    std::cerr << "Vehicle must be initialized with a path to follow (null).\n";
  }

  m_driverType = type;
  // set pointers to data dependencies
  physicsSystem = ps;
  m_navPath = pathToFollow;
  m_track = track;

  bool result;

  // Load the params from json or set directly.
  result = readBaseParamsFromJsonFile(gVehicleDataPath, "Base.json", m_Vehicle.mBaseParams);
  if (!result)
  {
    std::cerr << "Could not load vehicle engine params.\n";
    return; 
  }

  setPhysXIntegrationParams(m_Vehicle.mBaseParams.axleDescription,
                            physicsSystem->m_PhysXMaterialFrictions, physicsSystem->m_NbPhysXMaterialFrictions, physicsSystem->m_PhysXDefaultMaterialFriction,
                            m_Vehicle.mPhysXParams);

  result = readEngineDrivetrainParamsFromJsonFile(gVehicleDataPath, "EngineDrive.json",
                                         m_Vehicle.mEngineDriveParams);

  if (!result)
  {
    std::cerr << "Could not load drivetrain params.\n";
    return;
  }

  // Set the states to default.
  if (!m_Vehicle.initialize(*physicsSystem->m_Physics, PxCookingParams(PxTolerancesScale()), *physicsSystem->m_Material, EngineDriveVehicle::eDIFFTYPE_FOURWHEELDRIVE))
  {
    std::cerr << "Could not initialize a vehicle (physx init failed).\n";
    return;
  }

  // Apply a start pose to the physx actor and add it to the physx scene.
  //this is where the name of the car is applied to the actor
  m_Vehicle.setUpActor(*physicsSystem->m_Scene, initialPose, m_vehicleName);

  // Set the vehicle in 1st gear.
  m_Vehicle.mEngineDriveState.gearboxState.currentGear = m_Vehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
  m_Vehicle.mEngineDriveState.gearboxState.targetGear = m_Vehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;

  // Set the vehicle to use the automatic gearbox.
  m_Vehicle.mTransmissionCommandState.targetGear = PxVehicleEngineDriveTransmissionCommandState::eAUTOMATIC_GEAR;


  // Grabs the initial center of mass to be able to restore it later
  c_mass_init_v = m_Vehicle.mPhysXParams.physxActorCMassLocalPose;
  angular_damp_init_v = m_Vehicle.mPhysXState.physxActor.rigidBody->getAngularDamping();

  PxU32 vehicle_shapes = m_Vehicle.mPhysXState.physxActor.rigidBody->getNbShapes();
  for (PxU32 i = 0; i < vehicle_shapes; i++)
  {
      PxShape* shape = NULL;
      m_Vehicle.mPhysXState.physxActor.rigidBody->getShapes(&shape, 1, i);
      shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
      shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
      shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
  }

 // setup with custom params
  setup1();

}


void Car::cleanupVehicle()
{
  m_Vehicle.destroy();
}

PxRigidBody* Car::getVehicleRigidBody()
{ 
  return m_Vehicle.mPhysXState.physxActor.rigidBody;
}

void Car::carImGui() {
    //ImGui::Begin("Car");
    if (ImGui::TreeNode("Debug Readouts")) {        
        ImGui::Text("left stick horizontal tilt: %f", carAxis);
        //ImGui::Text("Car Throttle: %f", controller_throttle);
        //ImGui::Text("Car Brake: %f", controller_brake);
        ImGui::Text("Car Location: %f, %f", m_Vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p.x, m_Vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p.z);
        ImGui::Text("Current Gear: %d", m_Vehicle.mEngineDriveState.gearboxState.currentGear);
        ImGui::Text("Current engine rotational speed: %f", m_Vehicle.mEngineDriveState.engineState.rotationSpeed);
        ImGui::Text("Center of Gravity: %f, %f, %f", m_Vehicle.mPhysXParams.physxActorCMassLocalPose.p.x,
            m_Vehicle.mPhysXParams.physxActorCMassLocalPose.p.y,
            m_Vehicle.mPhysXParams.physxActorCMassLocalPose.p.z);
        ImGui::Text("Suspension force x: %f", m_Vehicle.mBaseState.suspensionForces->force.x);
        ImGui::Text("Suspension force y: %f", m_Vehicle.mBaseState.suspensionForces->force.y);
        ImGui::Text("Suspension force z: %f", m_Vehicle.mBaseState.suspensionForces->force.z);
        ImGui::Text("Rotation x: %f, y: %f, z: %f", m_Vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.x, m_Vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.y,
            m_Vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.z);
        ImGui::Text("On the ground ?: %s", m_Vehicle.mBaseState.roadGeomStates->hitState ? "true" : "false");
        ImGui::Text("Percent Rot: %f", 1.f - m_Vehicle.mEngineDriveState.engineState.rotationSpeed / m_Vehicle.mEngineDriveParams.engineParams.maxOmega);
        ImGui::Text("Steer Response: %f", m_Vehicle.mBaseParams.steerResponseParams.maxResponse);
        ImGui::Text("Linear Velocity: %f, %f, %f", m_Vehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity().x, m_Vehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity().y,
            m_Vehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity().z);
        auto vehicleRot = m_Vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q;
        ImGui::Text("Car rotation: %f, %f, %f, %f", vehicleRot.w, vehicleRot.x, vehicleRot.y, vehicleRot.z);
        ImGui::Text("Friction? %f, %f", m_Vehicle.mBaseState.tireSlipStates->slips[0], m_Vehicle.mBaseState.tireSlipStates->slips[1]);
        ImGui::Text("Is wrong way?: %s", isWrongWay() ? "true" : "false");
        ImGui::Text("Grounded?: %s", m_grounded ? "true" : "false");
        
        ImGui::TreePop();

        ImGui::SliderFloat("Strength of UP:", &STRENGTH_UP_CORRECTION, 1.0f, 10000);


    }
    if (ImGui::TreeNode("Parameter Switching")) {
        if (ImGui::Button("Default parameters")) {
            baseSetup();
        }
        if (ImGui::Button("Setup 1")) {
            setup1();
        }

        ImGui::TreePop();
    }

    //ImGui::End();
}

void Car::baseSetup() {
    m_Vehicle.mBaseParams.rigidBodyParams.mass = 1500.f;
    m_Vehicle.mBaseParams.rigidBodyParams.moi.x = 3200.0f;
    m_Vehicle.mBaseParams.rigidBodyParams.moi.y = 3414.0f;
    m_Vehicle.mBaseParams.rigidBodyParams.moi.z = 750.0f;

    m_Vehicle.mBaseParams.wheelParams[0].radius = 0.3432520031929016;
    m_Vehicle.mBaseParams.wheelParams[0].halfWidth = 0.15768450498580934;
    m_Vehicle.mBaseParams.wheelParams[1].radius = 0.3432520031929016;
    m_Vehicle.mBaseParams.wheelParams[1].halfWidth = 0.15768450498580934;
    m_Vehicle.mBaseParams.wheelParams[2].radius = 0.3432520031929016;
    m_Vehicle.mBaseParams.wheelParams[2].halfWidth = 0.15768450498580934;
    m_Vehicle.mBaseParams.wheelParams[3].radius = 0.3432520031929016;
    m_Vehicle.mBaseParams.wheelParams[3].halfWidth = 0.15768450498580934;

    m_Vehicle.mBaseParams.steerResponseParams.maxResponse = 0.52f;
    m_Vehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers[0] = 1.0f;
    m_Vehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers[1] = 1.0f;
    m_Vehicle.mBaseParams.tireForceParams->latStiffY = 143930.84033118f;

    m_Vehicle.mEngineDriveParams.engineParams.peakTorque = 500.f;
    m_Vehicle.mEngineDriveParams.engineParams.maxOmega = 600.f;
    m_Vehicle.mEngineDriveParams.autoboxParams.latency = 2.0f;
    m_Vehicle.mEngineDriveParams.gearBoxParams.switchTime = 0.5f;
}

void Car::setup1() {
    // For rigid body moi calculations
    // x= (L^2 + H^2)*M/12, y=(W^2+L^2)*M/12, z=(H^2+W^2)*M/12
    //width = 1.68
    //height = 1.3
    //length = 2.74
    m_Vehicle.mBaseParams.rigidBodyParams.mass = 500.f;
    m_Vehicle.mBaseParams.rigidBodyParams.moi.x = 3200;
    m_Vehicle.mBaseParams.rigidBodyParams.moi.y = 2000;
    m_Vehicle.mBaseParams.rigidBodyParams.moi.z = 3200;

    m_Vehicle.mBaseParams.wheelParams[0].radius = .6f;
    m_Vehicle.mBaseParams.wheelParams[0].halfWidth = .3f;
    m_Vehicle.mBaseParams.wheelParams[1].radius = .6f;
    m_Vehicle.mBaseParams.wheelParams[1].halfWidth = .3f;
    m_Vehicle.mBaseParams.wheelParams[2].radius = .6f;
    m_Vehicle.mBaseParams.wheelParams[2].halfWidth = .3f;
    m_Vehicle.mBaseParams.wheelParams[3].radius = .6f;
    m_Vehicle.mBaseParams.wheelParams[3].halfWidth = .3f;

    m_Vehicle.mBaseParams.steerResponseParams.maxResponse = 0.6f;
    m_Vehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers[0] = 0.5f;
    m_Vehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers[1] = 0.5f;
    m_Vehicle.mBaseParams.tireForceParams->latStiffY = 3186990.625f;

    m_Vehicle.mEngineDriveParams.engineParams.peakTorque = 500.f;
    m_Vehicle.mEngineDriveParams.engineParams.maxOmega = 600.f;
    m_Vehicle.mEngineDriveParams.autoboxParams.latency = 0.5f;
    m_Vehicle.mEngineDriveParams.gearBoxParams.switchTime = 0.2f;
}

void Car::setup2() {
    // For rigid body moi calculations
    // x= (L^2 + H^2)*M/12, y=(W^2+L^2)*M/12, z=(H^2+W^2)*M/12
    //width = 1.68
    //height = 1.3
    //length = 2.74
    m_Vehicle.mBaseParams.rigidBodyParams.mass = 500.f;
    m_Vehicle.mBaseParams.rigidBodyParams.moi.x = 3200;
    m_Vehicle.mBaseParams.rigidBodyParams.moi.y = 2000;
    m_Vehicle.mBaseParams.rigidBodyParams.moi.z = 3200;

    m_Vehicle.mBaseParams.wheelParams[0].radius = .6f;
    m_Vehicle.mBaseParams.wheelParams[0].halfWidth = .3f;
    m_Vehicle.mBaseParams.wheelParams[1].radius = .6f;
    m_Vehicle.mBaseParams.wheelParams[1].halfWidth = .3f;
    m_Vehicle.mBaseParams.wheelParams[2].radius = .6f;
    m_Vehicle.mBaseParams.wheelParams[2].halfWidth = .3f;
    m_Vehicle.mBaseParams.wheelParams[3].radius = .6f;
    m_Vehicle.mBaseParams.wheelParams[3].halfWidth = .3f;

    m_Vehicle.mBaseParams.steerResponseParams.maxResponse = 0.6f;
    m_Vehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers[0] = 0.5f;
    m_Vehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers[1] = 0.5f;
    m_Vehicle.mBaseParams.tireForceParams->latStiffY = 3186990.625f;

    m_Vehicle.mEngineDriveParams.engineParams.moi = 0.5;
    m_Vehicle.mEngineDriveParams.engineParams.peakTorque = 500.f;
    m_Vehicle.mEngineDriveParams.engineParams.maxOmega = 600.f;
    m_Vehicle.mEngineDriveParams.autoboxParams.latency = 0.5f;
    m_Vehicle.mEngineDriveParams.gearBoxParams.switchTime = 0.1f;
}



void Car::resetModifications() {
    // ORIGINALLY MEANT TO RESET THE CENTER OF GRAVITY, BUT WORKS BETTER WITHOUT CHANGING ?
    m_Vehicle.mPhysXParams.physxActorCMassLocalPose.p = c_mass_init_v.p;
    m_Vehicle.mPhysXState.physxActor.rigidBody->setAngularDamping(angular_damp_init_v);
}

PxTransform closest_tether_point;
void Car::setClosestTetherPoint(PxTransform _loc) {
    closest_tether_point = _loc;
}
void Car::setClosestTetherPoint(glm::vec3 _loc) {
    closest_tether_point.p.x = _loc.x;
    closest_tether_point.p.y = _loc.y;
    closest_tether_point.p.z = _loc.z;
}

// Used to reset the modifications done to the car in the air after a few moments
// This exists because if I just check the hit state it will reset the modifications instantly
bool Car::isGroundedDelay(Car &car) {
    if (!car.m_Vehicle.mBaseState.roadGeomStates->hitState) {
        time_elapsed += 1;
        return false;
    }

    else if (time_elapsed > 10) {
        if (car.m_Vehicle.mBaseState.roadGeomStates->hitState) {
            time_elapsed = 0;
            has_jumped = false;
            return true;
        }
    }
    else {
        return false;
    }
}

bool Car::Jump() {
    // For modes
    //eIMPULSE
    // or
    //eVELOCITY_CHANGE

    if (m_Vehicle.mBaseState.roadGeomStates->hitState) {
        // Caution force is proportional to the mass of the car, the lower the mass, the harder the force will be applied
        // TODO:: Make a function to calculate approriate force to be passed based on vehicle mass
        m_Vehicle.mPhysXState.physxActor.rigidBody->addForce(PxVec3(0.f, 4000.f, 0.f), PxForceMode::eIMPULSE, true);
        // applying angular dampening prevents the car from rotating while in the air
        // Prevents the car from spinning around the y axis while in the air
        m_Vehicle.mPhysXState.physxActor.rigidBody->setAngularDamping(10000.f);
        //m_Vehicle.mPhysXState.physxActor.rigidBody->addTorque(PxVec3(0.f, 10.f, 0.f), PxForceMode::eVELOCITY_CHANGE, true);
    }
    return true;
}

// Made a different jump for the ai because it was going too high using the player function
// not sure why, this is a quick workaround
bool Car::AiJump() {
    // For modes
    //eIMPULSE
    // or
    //eVELOCITY_CHANGE

    if (m_Vehicle.mBaseState.roadGeomStates->hitState) {
        // Caution force is proportional to the mass of the car, the lower the mass, the harder the force will be applied
        // TODO:: Make a function to calculate approriate force to be passed based on vehicle mass
        m_Vehicle.mPhysXState.physxActor.rigidBody->addForce(PxVec3(0.f, 2500.f, 0.f), PxForceMode::eIMPULSE, true);
        // applying angular dampening prevents the car from rotating while in the air
        // Prevents the car from spinning around the y axis while in the air
        m_Vehicle.mPhysXState.physxActor.rigidBody->setAngularDamping(10000.f);
        //m_Vehicle.mPhysXState.physxActor.rigidBody->addTorque(PxVec3(0.f, 10.f, 0.f), PxForceMode::eVELOCITY_CHANGE, true);
    }
    return true;
}

void Car::BoostForward(float magnitude)
{
    // will boost the car forward !! 

    // get the heading direction of the car and scale it by the magnitude of the boost
    glm::vec3 _heading = getForwardDir();
    PxVec3 heading = GLMtoPx(_heading);

    // Caution force is proportional to the mass of the car, the lower the mass, the harder the force will be applied
    m_Vehicle.mPhysXState.physxActor.rigidBody->addForce(heading * magnitude, PxForceMode::eIMPULSE, true);
}



void Car::Update(Guid carGuid, ecs::Scene& scene, float deltaTime)
{

  float delta_seconds = deltaTime;
  assert(delta_seconds > 0.f && delta_seconds < 0.2000001f);  

  m_timeSinceLastBoost += delta_seconds;
  m_timeSinceLastRamp += delta_seconds;
  m_timeSinceLastJump += delta_seconds;

  Command command = drive(carGuid, scene, deltaTime);
  keepRigidbodyUpright(m_Vehicle.mPhysXState.physxActor.rigidBody);

  checkFlipped(getVehicleRigidBody()->getGlobalPose());

  PxTransform local = getTransformPx();
  // local down vector
  PxVec3 down(0.f,-1.f,0.f);

  bool obstacle_under{false};
  bool ramp_under{false};

  PxShape* hitShape = castRayCheckShape(physicsSystem->m_Scene, local.p, down, 10.f);
  // check for obstacles in front of the ai    
  if (hitShape != nullptr)
  {
    for (Guid entity : ecs::EntitiesInScene<ObstacleCollider>(scene))
    {
        // get the level collider
        ObstacleCollider& oc = scene.GetComponent<ObstacleCollider>(entity);
        obstacle_under = obstacle_under || (oc.getShape() == hitShape);
    }
    for (Guid entity : ecs::EntitiesInScene<RampCollider>(scene))
    {
        // get the level collider
        RampCollider& oc = scene.GetComponent<RampCollider>(entity);
        ramp_under = ramp_under || (oc.getShape() == hitShape);
    }

    // check if they should get boosted :p 
    if (obstacle_under)
    {
        std::cout << "boost detected!!!\n";
        if (m_timeSinceLastBoost > 1.5f)
        {
            BoostForward(4500.f);
            m_timeSinceLastBoost = 0.f;

        }
    }
    if (ramp_under)
    {
        std::cout << "ramp detected!!!\n";
        if (m_timeSinceLastRamp > 1.5f)
        {
            BoostForward(10000.f);
            m_timeSinceLastRamp = 0.f;
        }
    }

  }

  hitShape = castRayCheckShape(physicsSystem->m_Scene, local.p, local.q.rotate(down), 1.f);
  for (Guid entity : ecs::EntitiesInScene<RoadCollider>(scene))
  {
    RoadCollider& rc = scene.GetComponent<RoadCollider>(entity);
    m_grounded = (rc.getShape() == hitShape) ? true : false;
  }

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
  m_Vehicle.step(delta_seconds, physicsSystem->m_VehicleSimulationContext);

}


glm::vec3 Car::getForwardDir()
{
        // find rotation matrix of car
    PxTransform carPose = this->m_Vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose();

    // find the direction vector of the vehicle
    glm::quat vehicleQuat = PxtoGLM(carPose.q);
    glm::mat4 vehicleRotM = glm::toMat4(vehicleQuat);
    glm::vec3 headingDir = glm::vec3{vehicleRotM * glm::vec4{0.f, 0.f, 1.f, 1.f}};
    
    return glm::normalize(headingDir);
}


bool Car::isWrongWay()
{
    glm::vec3 forward = getForwardDir();
    glm::vec3 trackForward = m_track->forward(getPosition());

    float angle = acos(glm::dot(forward, trackForward));

    if (angle > M_PI_2)
    {
        std::cout << "CAR IS WRONG WAY!\n";
        return true;
    }
    return false;
}

void Car::checkFlipped(PxTransform carPose)
{

      // find the direction vector of the vehicle
    glm::quat vehicleQuat = PxtoGLM(carPose.q);
    glm::mat4 vehicleRotM = glm::toMat4(vehicleQuat);

    glm::vec3 upDir = glm::vec3{0.f,1.f,0.f};
    // add y components ???
    glm::vec3 carNormal = glm::vec3{vehicleRotM * glm::vec4{upDir, 1.f}};
    
    // std::cout << "ai car y direction: " <<  carNormal.y << std::endl;

    // TODO: incorporate a timer/ground check into this so that it's not wonky in midair?
    if (carNormal.y < 0.f) // if car's up vector is negative, it's flipped.
    {
        // flip the car!!
        carPose.q = PxQuat(physx::PxIDENTITY::PxIdentity);
        // need to subtract y components
        m_Vehicle.mPhysXState.physxActor.rigidBody->setGlobalPose(carPose);

        // Dampens the angular momentum so you don't keep flipping during reset
        m_Vehicle.mPhysXState.physxActor.rigidBody->setAngularDamping(10000.f);
    }
}

glm::vec3 Car::getPosition()
{
    PxTransform carPose = m_Vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose();
    return PxtoGLM(carPose.p);
}


PxTransform Car::getTransformPx()
{
    PxTransform carPose = m_Vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose();
    return carPose;
}

Command Car::drive(Guid carGuid, ecs::Scene& scene, float deltaTime)
{

    Command command = {0.f, 0.f, 0.f, m_TargetGearCommand};

    // now we have to get the controller mapping
    ControllerInput::controller;

    ProgressTracker& pt = scene.GetComponent<ProgressTracker>(carGuid);

    if (m_driverType == DriverType::COMPUTER || (m_driverType == HUMAN && pt.isFinished) )
    {
        return pathfind(scene,deltaTime);
    }
    else if (m_driverType == DriverType::HUMAN)
    {

        SDL_GameController* controller{nullptr};
        // get controller if it exists
        if (controllerIndex != -1)
        {
            // controller exists for player
            controller = ControllerInput::controllers[controllerIndex];
        }
        PxTransform carPose = m_Vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose();
        glm::vec3 glm_carPose = PxtoGLM(carPose.p);

        // else, get the human driver's movement.
        auto keys_arr = SDL_GetKeyboardState(nullptr);
        auto w_key = keys_arr[SDL_SCANCODE_W];
        auto s_key = keys_arr[SDL_SCANCODE_S];
        auto a_key = keys_arr[SDL_SCANCODE_A];
        auto d_key = keys_arr[SDL_SCANCODE_D];
        auto space_bar = keys_arr[SDL_SCANCODE_SPACE];
        auto m_key = keys_arr[SDL_SCANCODE_M];
        auto f_key = keys_arr[SDL_SCANCODE_F];

        // Jump
        // Checks if the previous frame was a jump, so that it does not cumulatively add impulse
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A) && this->m_Vehicle.mBaseState.roadGeomStates->hitState)
        {
            AiJump(); // Using ai jump as remove the 'has jumped' check would make the car jump super high
            // TODO:: replace aijump with jump and use the aijump values
        }

        // Normalize controller axis
        // BUG: max positive is 1 less in magnitude than max min meaning full negative will be slightly above 1

        carAxis = (float)-SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) / SHRT_MAX;

        // Controller deadzone to avoid controller drift when
        // stick is at rest
        if (carAxis < 0.2f && carAxis > -0.2f)
        {
            carAxis = 0.f;
        }

        // Calculate the magnitude of the linear velocity
        // Used to clamp the gear shift to reverse only when below a certain magnitude
        // If not using clamp - it would cause the car to go into reverse when hitting the brake, which would not
        // have the proper braking behaviour
        float linVelMagnitude = length(PxtoGLM(m_Vehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity()));

        // Code for going in reverse
        // If the brake key is pressed, while you are going slow, switch to reverse
        if (s_key && m_Vehicle.mEngineDriveState.gearboxState.currentGear != 0 && linVelMagnitude < 10.f)
        {
            this->m_TargetGearCommand = 0;
        }
        // While the gearbox is in reverse holding s goes backwards, hold w brakes
        else if (this->m_Vehicle.mEngineDriveState.gearboxState.currentGear == 0)
        {
            if (s_key)
            {
                command.throttle = carThrottle;
            }
            // If you are going slow and the w key is pressed switch to normal driving
            else if (w_key && linVelMagnitude < 10.f)
            {
                // 255 is eAUTOMATIC_GEAR
                this->m_TargetGearCommand = 255;
            }
            else if (w_key)
            {
                command.brake = carBrake;
            }
        }
        // If the engine in neutral or above, drive normally
        else if (this->m_Vehicle.mEngineDriveState.gearboxState.currentGear >= 1)
        {

            if (w_key)
            {
                command.throttle = carThrottle;
            }
            else if (s_key)
            {
                command.brake = carBrake;
            }
        }

        // Same reverse code as above but for controllers - bundling them in with the keyboard
        // doing keyboard or controller input do x - did not work great
        // So I separated them, there may be a cleaner way to do this
        if (SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) && m_Vehicle.mEngineDriveState.gearboxState.currentGear != 0 && linVelMagnitude < 10.f)
        {
            this->m_TargetGearCommand = 0;
        }
        else if (this->m_Vehicle.mEngineDriveState.gearboxState.currentGear == 0)
        {
            if (SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT))
            {
                command.throttle = (float)SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / SHRT_MAX;
            }
            // If the engine is idle and the w key is pressed switch to normal driving
            else if (SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) && linVelMagnitude < 10.f)
            {
                // 255 is eAUTOMATIC_GEAR
                this->m_TargetGearCommand = 255;
            }
            else if (SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT))
            {
                command.brake = (float)SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / SHRT_MAX;
            }
        }
        else if (this->m_Vehicle.mEngineDriveState.gearboxState.currentGear >= 1)
        {

            if (SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT))
            {
                command.throttle = (float)SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / SHRT_MAX;
            }
            else if (SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT))
            {
                command.brake = (float)SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / SHRT_MAX;
            }
        }


        // Keyboard Controls
        if (a_key)
        {
            command.steer = 1.f;
        }
        else if (d_key)
        {
            command.steer = -1.f;
        }
        // Controller axis
        else
        {
            command.steer = carAxis * carAxisScale;
        }

        if (f_key || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y))
        {
            checkFlipped(carPose);
        }

        return command;
    }

    return command;
}

// AI METHODS


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






Command Car::pathfind(ecs::Scene& scene, float deltaTime)
{

    assert(m_navPath != nullptr);

    Command command = {0.f, 0.f, 0.f, m_TargetGearCommand};

    bool didLap{false};

    // find rotation matrix of car
    PxTransform carPose = this->m_Vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose();
    glm::vec3 glm_carPose = PxtoGLM(carPose.p);

    glm::vec3 targetPos = m_navPath->getNextPoint(glm_carPose,didLap);

    glm::vec4 carLocalForward{0.f, 0.f, -1.f, 1.f};

    // find the direction vector of the vehicle
    glm::quat vehicleQuat = PxtoGLM(carPose.q);
    glm::mat4 vehicleRotM = glm::toMat4(vehicleQuat);
    glm::vec3 headingDir = glm::vec3{vehicleRotM * glm::vec4{0.f, 0.f, 1.f, 1.f}};
    
    // const int num_rays{15};
    std::vector<glm::vec3> steering_rays;

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

    for (Guid entity : ecs::EntitiesInScene<LevelCollider>(scene))
    {
        // get the level collider
        LevelCollider& lc = scene.GetComponent<LevelCollider>(entity);
        level_c = &lc;
        break;
    }

    bool forced_turn_left{false};
    bool forced_turn_right{false};
    bool hitting_wall{false};
    bool obstacle_ahead{false};

    bool obstacle_under{false};

    // split into left and right
    bool hit = castRay(physicsSystem->m_Scene, carPose.p, GLMtoPx(steering_rays[0]), 16.f, level_c->getShape());
    if (hit)
    {
        // ray hit forward left
        forced_turn_right = true;
    }

    hit = castRay(physicsSystem->m_Scene, carPose.p, GLMtoPx(steering_rays[1]), 16.f, level_c->getShape());
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


    // check for obstacles in front of the ai    
    for (Guid entity : ecs::EntitiesInScene<ObstacleCollider>(scene))
    {
        // get the level collider
        ObstacleCollider& oc = scene.GetComponent<ObstacleCollider>(entity);

        obstacle_ahead = obstacle_ahead ||  castRay(physicsSystem->m_Scene, carPose.p, GLMtoPx(headingDir), 20.f, oc.getShape());
    }

    if (obstacle_ahead)
    {
        // std::cout << "ai needs to jump!" << std::endl;

        // also check the car is going fast enough for a jump to make sense
        if (carSpeed() > 10.f)
        {
            AiJump();
        } else {
            // need to correct
            hitting_wall = true;
            m_stuckTimer = 10.f;
        }
    }


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
    if (abs(actualAngle) < 0.23f) // 30 degrees (rads)
    {
        command.steer = 0.0f;
    } else {
        
        // want to scale steering more gently based on the actual angle

        // compute direction to turn
        PxVec3 cross = GLMtoPx(headingDir).cross(targetDir);
        float turn_dir = (cross.y < 0) ? -1.f : 1.f;

        // float normalized "turning" angle


        if ( (abs(actualAngle) > M_PI / 3 ) && (forced_turn_left || forced_turn_right))
        {
            command.throttle = 0.80f;
            
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

            float maxAngle = M_PI / 4.f;  // higher max angle = closer AI follows path

            if (abs(actualAngle) < maxAngle) // if the turning angle is more gentle
            {
                // slow down to steer (to correct faster) threshold
                if (abs(actualAngle) > M_PI_2)
                {
                    command.throttle = 0.65f;
                }

                // use the heading direction as the target direction instead 
                {
                    auto targetDir = GLMtoPx(m_navPath->getDirectionVector(glm_carPose));

                    PxVec3 cross = GLMtoPx(headingDir).cross(targetDir);
                    float turn_dir = (cross.y < 0) ? -1.f : 1.f;

                    command.steer = -turn_dir; // flip the turn direction in reverse!

                    // if close to steering the right direction, make steering less powerful
                    command.steer = turn_dir * (actualAngle / maxAngle);
                    // command.throttle *= clamp((1-(actualAngle / maxAngle)), 0.8f,1.f);

                }        


            } else {
                command.steer = turn_dir;
            }
        }

    }

    if (m_stuckTimer > 0.7f && hitting_wall)
    {
        // reverse
        m_TargetGearCommand = 0; // put in reverse
        command.throttle = 0.6f;

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

float Car::carSpeed()
{
    // return the projection of the car's linear velocity on it's heading/forward dir

    glm::vec3 v = PxtoGLM(m_Vehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity());
    glm::vec3 u = getForwardDir();

    return glm::length(v);
}