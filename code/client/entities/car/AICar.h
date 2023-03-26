#pragma once
#include "Car.h"
#include "core/ecs.h"
#include <vector>
#include "glm/glm.hpp"
#include "nav/NavPath.h"

struct AICar : Car {

    NavPath* m_navPath;

    AICar() : Car() {}

    virtual void Update(Guid carGuid, ecs::Scene& scene, float deltaTime);

    void Initialize(NavPath* pathToFollow); 

    Command pathfind(glm::vec3 currentPosition, ecs::Scene& scene, float deltaTime);

private:
    float m_stuckTimer;

};