#pragma once

#include "glm/glm.hpp"
#include "core/ecs.h"
#include "nav/NavPath.h"
#include "../../systems/PhysicsSystem.h"
#include "Car.h"
#include <vector>

Guid spawnCar(DriverType type, ecs::Scene& scene, physics::PhysicsSystem* physicsSystem, glm::vec3 position, glm::vec3 direction, Curve* track, NavPath* navPath);

std::vector<glm::vec3> spawnpointsAlongAxis(int rows, int cols,float spread, glm::vec3 axis, glm::vec3 start);
