#pragma once
#include "systems/ecs.h"
#include "systems/GraphicsSystem.h"
#include "systems/components.h"

#include <vector>

struct AISystem : ecs::ISystem {

    AISystem(Guid rendererGuid) : m_rendererGuid(rendererGuid) {}

    void Update(ecs::Scene& scene, float deltaTime);
    // will find a path from start entity to end entity
    static std::vector<std::pair<glm::vec3, float>> generateNearby(glm::vec3 initial);

private:
    Guid m_rendererGuid;
};

struct PathfindingComponent {
    Guid targetEntity;
    PathfindingComponent() : targetEntity(0) {}
    PathfindingComponent(Guid t) : targetEntity(t) {}

};