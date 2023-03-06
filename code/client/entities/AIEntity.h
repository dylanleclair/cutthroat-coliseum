#pragma once

#include "glm/glm.hpp"

Guid spawnAIEntity(ecs::Scene& scene, physics::PhysicsSystem* physicsSystem, Guid entityToFollow, glm::vec3 position, NavPath* circlePath);