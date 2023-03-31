#include "LevelCollider.h"


// TODO: add a type enum to the collider instead of using subclasses.
// when raycasting, check the enum type.

void spawnObstacle(std::string modelName, ecs::Scene& scene, PhysicsSystem& ps, PxMaterial* material)
{
  CPU_Geometry geometry = CPU_Geometry();
	GraphicsSystem::importOBJ(geometry, modelName);

	Guid collider_e = scene.CreateEntity().guid;
	scene.AddComponent(collider_e, LevelCollider());
	ObstacleCollider& obs_collider = scene.GetComponent<ObstacleCollider>(collider_e);
	obs_collider.Initialize(geometry, ps);
	physx::PxTriangleMesh* obs_mesh = obs_collider.cookLevel(glm::scale(glm::mat4(1), glm::vec3(1.0)));
	obs_collider.initLevelRigidBody(obs_mesh, material);
}