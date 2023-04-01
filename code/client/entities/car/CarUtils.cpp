
#include "CarUtils.h"
#include "core/ecs.h"
#include "glm/glm.hpp"
#include "../../systems/GraphicsSystem.h"
#include "../../systems/PhysicsSystem.h"
#include <iostream>
#include "Car.h"

// will instantiate an AI car
Guid spawnCar(DriverType type, ecs::Scene& scene, physics::PhysicsSystem* physicsSystem, glm::vec3 position, Curve* track, NavPath* navPath)
{

	// all vehicles are entities that consists of the following components
	// - an Car (wrapper around the physx vehicle)
	// - a transform component
	// - a model renderer
  // if an ai, the additional components are added:
	// - a LineRender that renders the direction of the vehicle

	ecs::Entity aiDriver_e = scene.CreateEntity();
    
  scene.AddComponent(aiDriver_e.guid, Car());
	Car& aiCar = scene.GetComponent<Car>(aiDriver_e.guid);

  // initialize the physx vehicle, associated data.
  aiCar.Initialize(type, GLMtoPx(position), physicsSystem, track, navPath);

  // renderer
	RenderModel aiDriver_r = RenderModel();
	GraphicsSystem::importOBJ(aiDriver_r, "AI_beta_cart.obj");
	aiDriver_r.setModelColor(glm::linearRand(glm::vec3(0), glm::vec3(1)));
	scene.AddComponent(aiDriver_e.guid, aiDriver_r);
	TransformComponent aiDriver_t = TransformComponent(aiCar.getVehicleRigidBody());
	aiDriver_t.setPosition(glm::vec3(0, -.34f, 1.2)); // relative position to collider?
	aiDriver_t.setScale(glm::vec3(3.2f, 3.2f, 3.2f));
	scene.AddComponent(aiDriver_e.guid, aiDriver_t);
	CPU_Geometry testline = CPU_Geometry();
	testline.verts.push_back({0.f, 0.f, 0.f});
	testline.verts.push_back({0.f, 10.f, 0.f});

  // if AI, add direction renderer for debugging :D 
  if (type == DriverType::COMPUTER)
  {
    CPU_Geometry forward = CPU_Geometry();
    forward.verts.push_back({0.f, 0.f, 0.f});
    forward.verts.push_back({0.f, 0.f, 5.f});
    // render the forward direction of the AI
    RenderLine aiVehicleDirection = RenderLine(forward);
    aiVehicleDirection.setColor({0.0f,1.f,0.f});
    scene.AddComponent(aiDriver_e.guid, aiVehicleDirection);
  }

  return aiDriver_e.guid; 

}
