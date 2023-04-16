#pragma once
#include "MeshCollider.h"

using namespace physics;

struct LevelCollider : MeshCollider {};

struct RoadCollider : MeshCollider {};

struct ObstacleCollider : MeshCollider{};

struct RampCollider : MeshCollider{};

void spawnMeshCollider(std::string modelName, ecs::Scene& scene, PhysicsSystem& ps, PxMaterial* material);