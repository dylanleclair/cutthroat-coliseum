#include "Car.h"
#include "Input.h"


const char* gVehicleDataPath = "vehicledata";

// // HACK(beau): make these visible to tuning imgui panel
float carThrottle = 1.f;
float carBrake = 1.f;
float carAxis = 0.f;
float carAxisScale = 1.f;

float controller_throttle = 0.f;
float controller_brake = 0.f;

PxTransform c_mass_init_v;
PxReal angular_damp_init_v;

bool Car::initVehicle()
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
  PxTransform pose(PxVec3(0.000000000f, -0.0500000119f, -1.59399998f), PxQuat(PxIdentity));
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

void Car::resetModifications() {
    // ORIGINALLY MEANT TO RESET THE CENTER OF GRAVITY, BUT WORKS BETTER WITHOUT CHANGING ?
    m_Vehicle.mPhysXParams.physxActorCMassLocalPose.p = c_mass_init_v.p;
    m_Vehicle.mPhysXState.physxActor.rigidBody->setAngularDamping(angular_damp_init_v);
}

// Very jank right now as you can spam it
void Car::TetherSteer(PxTransform _loc) {
    m_Vehicle.mPhysXParams.physxActorCMassLocalPose = _loc;

    //m_Vehicle.mPhysXState.physxActor.rigidBody->addForce(PxVec3(0.f, 0.f, 10.f), PxForceMode::eVELOCITY_CHANGE, true);
    //m_Vehicle.mPhysXState.physxActor.rigidBody->addTorque(PxVec3(0.f, -2.f, 0.f), PxForceMode::eVELOCITY_CHANGE, true);
}

void Car::TetherJump() {
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
}


void Car::Update(float deltaTime)
{

  auto keys_arr = SDL_GetKeyboardState(nullptr);
  auto w_key = keys_arr[SDL_SCANCODE_W];
  auto s_key = keys_arr[SDL_SCANCODE_S];
  auto a_key = keys_arr[SDL_SCANCODE_A];
  auto d_key = keys_arr[SDL_SCANCODE_D];
  auto space_bar = keys_arr[SDL_SCANCODE_SPACE];

  float delta_seconds = deltaTime;
  assert(delta_seconds > 0.f && delta_seconds < 0.2000001f);
  // Apply the brake, throttle and steer to the command state of the vehicle.
  // const Command &command = gCommands[gCommandProgress];

  Command command = {0.f, 0.f, 0.f, m_TargetGearCommand};
  // command.duration = timestep;

  if (SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT)) {
      controller_throttle = (float)SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / SHRT_MAX;
      command.throttle = controller_throttle;      
  }
  if (SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT)) {
      controller_brake = (float)SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / SHRT_MAX;
      command.brake = controller_brake;      
  }

  // Currently a bit bugged, sends you into space
  // It's because it triggers every time it registers the button is pressed (held down)
  if (SDL_GameControllerGetButton(ControllerInput::controller, SDL_CONTROLLER_BUTTON_A)) {
      TetherJump();
  }

  // Normalize controller axis
  // BUG: max positive is 1 less in magnitude than max min meaning full negative will be slightly above 1
  carAxis = (float)-SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_LEFTX) / SHRT_MAX;
   

  // Keyboard Controls
  // Throttle to 2.f will cause weird behaviour
  if (w_key)
  {
      command.throttle = carThrottle;
  }
  else if (s_key)
  {
      command.brake = carBrake;
  }
  if (a_key)
  {
      command.steer = 1.f;
  }
  else if (d_key)
  {
      command.steer = -1.f;
  }
  else
  {
      command.steer = carAxis * carAxisScale;
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
  m_Vehicle.step(delta_seconds, m_VehicleSimulationContext);

  // Forward integrate the phsyx scene by a single timestep.
  physicsSystem->m_Scene->simulate(delta_seconds);
  physicsSystem->m_Scene->fetchResults(true);
}