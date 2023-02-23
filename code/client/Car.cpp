#include "Car.h"
#include "Input.h"


const char* gVehicleDataPath = "vehicledata";

// // HACK(beau): make these visible to tuning imgui panel
float carThrottle = 1.f;
float carBrake = 1.f;
float carAxis = 0.f;
float carAxisScale = 1.f;

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

void Car::stepPhysics(Timestep timestep)
{
        
    auto keys_arr = SDL_GetKeyboardState(nullptr);
    auto w_key = keys_arr[SDL_SCANCODE_W];
    auto s_key = keys_arr[SDL_SCANCODE_S];
    auto a_key = keys_arr[SDL_SCANCODE_A];
    auto d_key = keys_arr[SDL_SCANCODE_D];

    float delta_seconds = timestep.getSeconds();
    assert(delta_seconds > 0.f && delta_seconds < 0.2000001f);
  // Apply the brake, throttle and steer to the command state of the vehicle.
  // const Command &command = gCommands[gCommandProgress];

  Command command = {0.f, 0.f, 0.f, m_TargetGearCommand};
  // command.duration = timestep;

  // Throttle to 2.f will cause weird behaviour
  if (SDL_GameControllerGetButton(ControllerInput::controller, SDL_CONTROLLER_BUTTON_A) || w_key)
  {
      command.throttle = carThrottle;
    //goto end; // so we don't attempt to throttle and break
  }
  else if (SDL_GameControllerGetButton(ControllerInput::controller, SDL_CONTROLLER_BUTTON_B) || s_key)
  {
      command.brake = carBrake;
    // goto end;????
  }
//end:

  // Normalize controller axis
  // BUG: max positive is 1 less in magnitude than max min meaning full negative will be slightly above 1
  carAxis = (float) - SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_LEFTX) / SHRT_MAX;
  //std::cout << axis << std::endl;
  if (a_key) {
      command.steer = 1.f;
  } else if (d_key) {
      command.steer = -1.f;
  }
  else {
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

void Car::Update(float deltaTime)
{

  auto keys_arr = SDL_GetKeyboardState(nullptr);
  auto w_key = keys_arr[SDL_SCANCODE_W];
  auto s_key = keys_arr[SDL_SCANCODE_S];
  auto a_key = keys_arr[SDL_SCANCODE_A];
  auto d_key = keys_arr[SDL_SCANCODE_D];

  float delta_seconds = deltaTime;
  assert(delta_seconds > 0.f && delta_seconds < 0.2000001f);
  // Apply the brake, throttle and steer to the command state of the vehicle.
  // const Command &command = gCommands[gCommandProgress];

  Command command = {0.f, 0.f, 0.f, m_TargetGearCommand};
  // command.duration = timestep;

  // Throttle to 2.f will cause weird behaviour
  if (SDL_GameControllerGetButton(ControllerInput::controller, SDL_CONTROLLER_BUTTON_A) || w_key)
  {
      command.throttle = carThrottle;
      // goto end; // so we don't attempt to throttle and break
  }
  else if (SDL_GameControllerGetButton(ControllerInput::controller, SDL_CONTROLLER_BUTTON_B) || s_key)
  {
      command.brake = carBrake;
      // goto end;????
  }
  // end:

  // Normalize controller axis
  // BUG: max positive is 1 less in magnitude than max min meaning full negative will be slightly above 1
  carAxis = (float)-SDL_GameControllerGetAxis(ControllerInput::controller, SDL_CONTROLLER_AXIS_LEFTX) / SHRT_MAX;
  // std::cout << axis << std::endl;
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