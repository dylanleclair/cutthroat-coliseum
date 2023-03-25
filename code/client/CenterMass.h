#pragma once

#include "core/ecs.h"
#include "PxPhysicsAPI.h"
#include "systems/components.h"
#include "entities/car/Car.h"

using namespace physx;

void renderCMassSphere(TransformComponent& _target, TransformComponent& sphere_transform);
void renderCMassSphere(PxTransform& _target, TransformComponent& sphere_transform);
void setupSphere(ecs::Scene& mainScene, Car& car);