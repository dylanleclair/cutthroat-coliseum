#include "systems/ecs.h"
#include "glm/glm.hpp"
#include "systems/GraphicsSystem.h"
#include "systems/pathfinding.h"
#include "../systems/PhysicsSystem.h"
#include "../AICar.h"

Guid spawnAIEntity(ecs::Scene& scene, physics::PhysicsSystem* physicsSystem, Guid entityToFollow, glm::vec3 position, NavPath* circlePath)
{

	// an ai driver is an entity that consists of the following components
	// - an AICar (physx vehicle)
	// - a transform component
	// - a model renderer
	// - a LineRender that renders the direction of the vehicle

	ecs::Entity aiDriver_e = scene.CreateEntity();
    
    scene.AddComponent(aiDriver_e.guid, AICar());
	AICar& aiCar = scene.GetComponent<AICar>(aiDriver_e.guid);
	aiCar.physicsSystem = physicsSystem;
	aiCar.Initialize(circlePath);
	if (!aiCar.initVehicle(GLMtoPx(position)))
	{
		std::cout << "ERROR: could not initialize ai-driven vehicle";
	}

	// AI car entity setup
	RenderModel aiDriver_r = RenderModel();
	GraphicsSystem::importOBJ(aiDriver_r, "test_car.obj");
	aiDriver_r.setModelColor(glm::vec3(1.0f, 0.0f, 1.f));
	scene.AddComponent(aiDriver_e.guid, aiDriver_r);
	TransformComponent aiDriver_t = TransformComponent(aiCar.getVehicleRigidBody());
	aiDriver_t.setPosition(glm::vec3(0, 0, 1.2)); // relative position to collider?
	aiDriver_t.setScale(glm::vec3(3.2f, 3.2f, 3.2f));
	scene.AddComponent(aiDriver_e.guid, aiDriver_t);
	CPU_Geometry testline = CPU_Geometry();
	testline.verts.push_back({0.f, 0.f, 0.f});
	testline.verts.push_back({0.f, 10.f, 0.f});
	CPU_Geometry forward = CPU_Geometry();
	forward.verts.push_back({0.f, 0.f, 0.f});
	forward.verts.push_back({0.f, 0.f, 5.f});
	RenderLine aiVehicleDirection = RenderLine(forward);
	aiVehicleDirection.setColor({0.0f,1.f,0.f});
	scene.AddComponent(aiDriver_e.guid, aiVehicleDirection);
	scene.AddComponent(aiDriver_e.guid, PathfindingComponent{entityToFollow});
	// ecs::Entity aiDirRenderer = scene.CreateEntity();
	// scene.AddComponent(aiDirRenderer.guid, aiVehicleDirection);
	// scene.AddComponent(aiDirRenderer.guid, TransformComponent{});

    return aiDriver_e.guid; 

}