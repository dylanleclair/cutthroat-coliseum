#pragma once

#include "glm/glm.hpp"
#include "core/ecs.h"
#include "nav/NavPath.h"
#include "../../systems/PhysicsSystem.h"
#include "Car.h"

Guid spawnCar(DriverType type, ecs::Scene& scene, physics::PhysicsSystem* physicsSystem, glm::vec3 position, Curve* track, NavPath* navPath);