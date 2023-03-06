#pragma once
#include "systems/ecs.h"
#include "systems/GraphicsSystem.h"
#include "systems/components.h"
#include "PxPhysicsAPI.h"
#include "systems/PhysicsSystem.h"

using namespace physx;

void setUpLogs(ecs::Scene& mainScene);
void addRigidBody(physics::PhysicsSystem physicsSystem);