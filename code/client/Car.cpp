#include "Car.h"
#include "Input.h"
#include "glm/glm.hpp"
#include "utils/PxConversionUtils.h"


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
bool c_tethered = false;
bool previous_b_press = false;
PxTransform closest_tether_point;

PxTransform c_mass_init_v;
PxReal angular_damp_init_v;

bool Car::getCTethered() {
    return c_tethered;
}

bool Car::initVehicle(PxVec3 initialPosition)

{

  if (!physicsSystem)
  {
    std::cerr << "no physics system initialized.";
  }
    // // Check that we can read from the json file before continuing.
    // BaseVehicleParams baseParams;
    // if (!readBaseParamsFromJsonFile(gVehicleDataPath, "Base.json", baseParams))
    //     return false;

    // // Check that we can read from the json file before continuing.
    // EngineDrivetrainParams engineDrivetrainParams;
    // if (!readEngineDrivetrainParamsFromJsonFile(gVehicleDataPath, "EngineDrive.json",
    //                                             engineDrivetrainParams))
    // {
    //     return false;
    // }

  bool result;

  // Load the params from json or set directly.
  result = readBaseParamsFromJsonFile(gVehicleDataPath, "Base.json", m_Vehicle.mBaseParams);
  if (!result)
  {
    return false;
  }

  setPhysXIntegrationParams(m_Vehicle.mBaseParams.axleDescription,
                            physicsSystem->m_PhysXMaterialFrictions, physicsSystem->m_NbPhysXMaterialFrictions, physicsSystem->m_PhysXDefaultMaterialFriction,
                            m_Vehicle.mPhysXParams);

  result = readEngineDrivetrainParamsFromJsonFile(gVehicleDataPath, "EngineDrive.json",
                                         m_Vehicle.mEngineDriveParams);

  if (!result) return false;

  // Set the states to default.
  if (!m_Vehicle.initialize(*physicsSystem->m_Physics, PxCookingParams(PxTolerancesScale()), *physicsSystem->m_Material, EngineDriveVehicle::eDIFFTYPE_FOURWHEELDRIVE))
  {
    return false;
  }

  // Apply a start pose to the physx actor and add it to the physx scene.
  PxTransform pose(initialPosition, PxQuat(PxIdentity));
  m_Vehicle.setUpActor(*physicsSystem->m_Scene, pose, m_vehicleName);

  // Set the vehicle in 1st gear.
  m_Vehicle.mEngineDriveState.gearboxState.currentGear = m_Vehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
  m_Vehicle.mEngineDriveState.gearboxState.targetGear = m_Vehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;

  // Set the vehicle to use the automatic gearbox.
  m_Vehicle.mTransmissionCommandState.targetGear = PxVehicleEngineDriveTransmissionCommandState::eAUTOMATIC_GEAR;

  // Set up the simulation context.
  // The snippet is set up with
  // a) z as the longitudinal axis
  // b) x as the lateral axis
  // c) y as the vertical axis.
  // d) metres  as the lengthscale.
  m_VehicleSimulationContext.setToDefault();
  m_VehicleSimulationContext.frame.lngAxis = PxVehicleAxes::ePosZ;
  m_VehicleSimulationContext.frame.latAxis = PxVehicleAxes::ePosX;
  m_VehicleSimulationContext.frame.vrtAxis = PxVehicleAxes::ePosY;
  m_VehicleSimulationContext.scale.scale = 1.0f;
  m_VehicleSimulationContext.gravity = physicsSystem->m_Gravity;
  m_VehicleSimulationContext.physxScene = physicsSystem->m_Scene;
  m_VehicleSimulationContext.physxActorUpdateMode = PxVehiclePhysXActorUpdateMode::eAPPLY_ACCELERATION;

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

  return true;

}

void Car::cleanupVehicle()
{
  m_Vehicle.destroy();
}

PxRigidBody* Car::getVehicleRigidBody()
{  
  return m_Vehicle.mPhysXState.physxActor.rigidBody;
}

void Car::setClosestTetherPoint(PxTransform _loc) {
    closest_tether_point = _loc;
}
void Car::setClosestTetherPoint(glm::vec3 _loc) {
    closest_tether_point.p.x = _loc.x;
    closest_tether_point.p.y = _loc.y;
    closest_tether_point.p.z = _loc.z;
}

void Car::resetModifications() {
    // ORIGINALLY MEANT TO RESET THE CENTER OF GRAVITY, BUT WORKS BETTER WITHOUT CHANGING ?
    m_Vehicle.mPhysXParams.physxActorCMassLocalPose.p = c_mass_init_v.p;
    m_Vehicle.mPhysXState.physxActor.rigidBody->setAngularDamping(angular_damp_init_v);
    c_tethered = false;
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

// Very jank right now as you can spam it
void Car::TetherSteer(PxTransform _loc) {
    c_tethered = true;
    carAxis = 0.f;
    m_Vehicle.mPhysXParams.physxActorCMassLocalPose = _loc;

    //m_Vehicle.mPhysXState.physxActor.rigidBody->addForce(PxVec3(0.f, 0.f, 10.f), PxForceMode::eVELOCITY_CHANGE, true);
    m_Vehicle.mPhysXState.physxActor.rigidBody->addTorque(PxVec3(0.f, -1.5f, 0.f), PxForceMode::eVELOCITY_CHANGE, true);
}

bool Car::TetherJump() {
    auto v_pos = m_Vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose();
    // For modes
    //eIMPULSE
    // or
    //eVELOCITY_CHANGE

    // if v_pos.p.y is to prevent jumping if the car is already in the air
    // This is a very messy way of doing this - there might be a flag for if the car is in the air
    if (v_pos.p.y < 2.0f) {
        m_Vehicle.mPhysXState.physxActor.rigidBody->addForce(PxVec3(0.f, 20000.f, 0.f), PxForceMode::eIMPULSE, true);
        // SEEMS TO CURRENTLY WORK BETTER WITHOUT CHANGING THE CENTER OF GRAVITY
        //m_Vehicle.mPhysXParams.physxActorCMassLocalPose.p.y = -30.0f;
        //m_Vehicle.mPhysXParams.physxActorCMassLocalPose.p.z = 0.0f;
        // applying angular dampening prevents the car from rotating while in the air
        // it will prevent the car from turning when landing however
        m_Vehicle.mPhysXState.physxActor.rigidBody->setAngularDamping(20.f); 
        //m_Vehicle.mPhysXState.physxActor.rigidBody->addTorque(PxVec3(0.f, 10.f, 0.f), PxForceMode::eVELOCITY_CHANGE, true);
    }
    return true;
}


void Car::Update(Guid carGuid, ecs::Scene& scene, float deltaTime)
{

  auto keys_arr = SDL_GetKeyboardState(nullptr);
  auto w_key = keys_arr[SDL_SCANCODE_W];
  auto s_key = keys_arr[SDL_SCANCODE_S];
  auto a_key = keys_arr[SDL_SCANCODE_A];
  auto d_key = keys_arr[SDL_SCANCODE_D];
  auto space_bar = keys_arr[SDL_SCANCODE_SPACE];
  auto m_key = keys_arr[SDL_SCANCODE_M];

  float delta_seconds = deltaTime;
  assert(delta_seconds > 0.f && delta_seconds < 0.2000001f);
  // Apply the brake, throttle and steer to the command state of the vehicle.
  // const Command &command = gCommands[gCommandProgress];

  Command command = {0.f, 0.f, 0.f, m_TargetGearCommand};
  // command.duration = timestep;

  // Jump tether
  // Checks if the previous frame was a jump, so that it does not cumulatively add impulse
  if (SDL_GameControllerGetButton(ControllerInput::controller, SDL_CONTROLLER_BUTTON_A) && this->m_Vehicle.mBaseState.roadGeomStates->hitState && !has_jumped) {
      if (TetherJump()) {
          has_jumped = true;
      }
  }

  // The behaviour of this is - when you hold down the button it will steer you around corners
  // Until you release the button to reset and give control back to you
  if (SDL_GameControllerGetButton(ControllerInput::controller, SDL_CONTROLLER_BUTTON_B) && !previous_b_press) {
      if (!c_tethered) {
          c_tethered = true;
          previous_b_press = true;
          TetherSteer(closest_tether_point);
      }
  }
  else if (!SDL_GameControllerGetButton(ControllerInput::controller, SDL_CONTROLLER_BUTTON_B) && previous_b_press) {
      resetModifications();
      previous_b_press = false;
  }

  // Normalize controller axis
  // BUG: max positive is 1 less in magnitude than max min meaning full negative will be slightly above 1
  if (!c_tethered) {
      carAxis = (float)-SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_LEFTX) / SHRT_MAX;
  }
  
  // Code for going in reverse
  // If the brake key is pressed, while the engine is idle, and the current gear is first gear, switch to reverse
  if (s_key && this->m_Vehicle.mEngineDriveState.gearboxState.currentGear == 2 &&
      this->m_Vehicle.mEngineDriveState.engineState.rotationSpeed == 0) {
      this->m_TargetGearCommand = 0;
  }
  // While the gearbox is in reverse holding s goes backwards, hold w brakes
  else if (this->m_Vehicle.mEngineDriveState.gearboxState.currentGear == 0) {
      if (s_key) {
          command.throttle = carThrottle;
      }
      // If the engine is idle and the w key is pressed switch to normal driving
      else if (w_key && this->m_Vehicle.mEngineDriveState.engineState.rotationSpeed == 0) {
           this->m_TargetGearCommand = 2;
      }
      else if (w_key) {
          command.brake = carBrake;
      }
  }
  // If the engine in neutral or above, drive normally
  else if (this->m_Vehicle.mEngineDriveState.gearboxState.currentGear >= 1 && 
      this->m_Vehicle.mEngineDriveState.engineState.rotationSpeed >= 0) {

       if (w_key) {
          command.throttle = carThrottle;
       }
       else if (s_key) {
           command.brake = carBrake;
       }
  }

  // Same reverse code as above but for controllers - bundling them in with the keyboard
  // doing keyboard or controller input do x - did not work great
  // So I separated them, there may be a cleaner way to do this
  if (SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT)
      && this->m_Vehicle.mEngineDriveState.gearboxState.currentGear == 2 &&
      this->m_Vehicle.mEngineDriveState.engineState.rotationSpeed == 0) {
      this->m_TargetGearCommand = 0;
  }
  else if (this->m_Vehicle.mEngineDriveState.gearboxState.currentGear == 0) {
      if (SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT)) {
          command.throttle = (float)SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / SHRT_MAX;
      }
      // If the engine is idle and the w key is pressed switch to normal driving
      else if (SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
          && this->m_Vehicle.mEngineDriveState.engineState.rotationSpeed == 0) {
          this->m_TargetGearCommand = 2;
      }
      else if (SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT)) {
          command.brake = (float)SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / SHRT_MAX;
      }
  }
  else if (this->m_Vehicle.mEngineDriveState.gearboxState.currentGear >= 1 &&
      this->m_Vehicle.mEngineDriveState.engineState.rotationSpeed >= 0) {

      if (SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT)) {
          command.throttle = (float)SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / SHRT_MAX;
      }
      else if (SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT)) {
          command.brake = (float)SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / SHRT_MAX;
      }
  }



  // Keyboard Controls
  if (a_key && !c_tethered)
  {
      command.steer = 1.f;
  }
  else if (d_key && !c_tethered)
  {
      command.steer = -1.f;
  }
  // Controller axis
  else
  {
      command.steer = carAxis * carAxisScale;
  }

  // An attempt at replicating the b face button function
  // of being able to hold down to active once, and let to go deactive
  // but for the keyboard it triggers every single frame it's being held down
  // 
  //if (m_key && !previous_b_press) {
  //    if (!c_tethered) {
  //        c_tethered = true;
  //        previous_b_press = true;
  //        TetherSteer(closest_tether_point);
  //    }
  //}
  //else if (!m_key && previous_b_press) {
  //    resetModifications();
  //   previous_b_press = false;
  //}
  

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
    }
}