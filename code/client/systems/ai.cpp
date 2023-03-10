#include "ai.h"
#include "core/pathfinding.h"
#include "core/ecs.h"
#include "GraphicsSystem.h"
#include "components.h"


glm::vec3 roundPosition(glm::vec3 pos)
{
    return glm::vec3(floor(pos.x), 0.f, floor(pos.z));
}

float euclideanXZ(glm::vec3 a, glm::vec3 b)
{
    float deltaX = b.x - a.x;
    float deltaZ = b.z - a.z;

    float dist = pow(deltaX, 2) + pow(deltaZ, 2);
    return sqrt(dist);
}

void AISystem::Update(ecs::Scene& scene, float deltaTime) {

    // // foil out the nodes !
    // CPU_Geometry pathGeom;

    // for (auto entityGuid : ecs::EntitiesInScene<PathfindingComponent>(scene))
    // {
    //     // get the pathfinding component
    //     PathfindingComponent& p = scene.GetComponent<PathfindingComponent>(entityGuid);
    //     TransformComponent& position = scene.GetComponent<TransformComponent>(entityGuid);
    //     // find the target
    //     Guid targetEntity = p.targetEntity;

    //     if (scene.HasComponent<TransformComponent>(targetEntity))
    //     {
    //         TransformComponent targetPosition = scene.GetComponent<TransformComponent>(targetEntity);
    //         std::vector<glm::vec3> path = pathfinding::AStar<glm::vec3>(roundPosition(position.getTranslation()), roundPosition(targetPosition.getTranslation()), euclideanXZ, AISystem::generateNearby);

    //         // std::vector<glm::vec3> path = {glm::vec3{0.f},glm::vec3{0.5f}, glm::vec3{0.6f}};
    //         for (auto& pos : path)
    //         {
    //             pathGeom.verts.push_back(glm::vec3{ pos });
    //         }
    //     } 
    // }

    // // want some way to render the path!
    // scene.GetComponent<RenderLine>(this->m_rendererGuid).setGeometry(pathGeom);
    // scene.GetComponent<RenderLine>(this->m_rendererGuid).setColor(glm::vec3(1,0,0));
}

std::vector<std::pair<glm::vec3, float>> AISystem::generateNearby(glm::vec3 initial)
{
    std::vector<std::pair<glm::vec3, float>> output;
    // we will only go along z plane for now
    glm::vec3 current{ initial };


    // four cardinal directions (on x-z plane)
    output.push_back({ { initial.x + 1, initial.y, initial.z }, 1 });
    output.push_back({ { initial.x - 1, initial.y, initial.z }, 1 });
    output.push_back({ { initial.x, initial.y, initial.z + 1 }, 1 });
    output.push_back({ { initial.x, initial.y, initial.z - 1}, 1 });

    // the other directions! (diagonals on x-z plane)
    output.push_back({ { initial.x + 1, initial.y, initial.z + 1 }, sqrt(2.f) });
    output.push_back({ { initial.x - 1, initial.y, initial.z - 1}, sqrt(2.f) });
    output.push_back({ { initial.x + 1, initial.y, initial.z - 1 }, sqrt(2.f) });
    output.push_back({ { initial.x - 1, initial.y, initial.z + 1}, sqrt(2.f) });

    return output;
}