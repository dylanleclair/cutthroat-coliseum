#pragma once
#include "core/ecs.h"
#include "../../systems/GraphicsSystem.h"
#include "../../systems/components.h"
#include "../../systems/PhysicsSystem.h"
#include "PxPhysicsAPI.h"

using namespace physx;

void setUpLogs(ecs::Scene& mainScene);
void addRigidBody(physics::PhysicsSystem physicsSystem);