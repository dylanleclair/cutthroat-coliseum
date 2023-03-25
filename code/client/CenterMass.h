#pragam once

#include "core/ecs.h"
#include "PxPhysicsAPI.h"
#include "systems/components.h"

void renderCMassSphere(TransformComponent& _target, TransformComponent& sphere_transform);
void renderCMassSphere(PxTransform& _target, TransformComponent& sphere_transform);
void setupSphere(ecs::Scene& mainScene);