
#include "CarUtils.h"
#include "core/ecs.h"
#include "glm/glm.hpp"
#include "../../systems/GraphicsSystem.h"
#include "../../systems/PhysicsSystem.h"
#include <iostream>
#include "Car.h"
#include "../../utils/PxConversionUtils.h"
#include "../../systems/RaceSystem.h"

#include <algorithm>
#include <random>

// will instantiate an AI car
Guid spawnCar(DriverType type, ecs::Scene& scene, physics::PhysicsSystem* physicsSystem, glm::vec3 position, glm::vec3 forward, Curve* track, NavPath* navPath, std::string name)
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

	// set rotation of the car.
	glm::quat q = quatLookAt(forward, {0,1.f,0});
	PxQuat initialRotation = GLMtoPx(q);
	PxVec3 initialPosition = GLMtoPx(position);

  PxTransform pose(initialPosition,initialRotation);

  // initialize the physx vehicle, associated data.
  aiCar.Initialize(type, pose, physicsSystem, track, navPath, name);
	
  // renderer
	RenderModel aiDriver_r = RenderModel();


	// Currently all cars are set to computers first and then dynamically set to players
	// based on controller count, so all carts will load the same model
	if (type == DriverType::COMPUTER)
	{
			GraphicsSystem::importOBJ(aiDriver_r, "beta_cart.obj");
	} else {
			GraphicsSystem::importOBJ(aiDriver_r, "beta_cart.obj");
	}

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

	scene.AddComponent(aiDriver_e.guid,ProgressTracker{aiDriver_e.guid});



	// add nameplates

	ecs::Entity nameplate = scene.CreateEntity();
	TransformComponent nameplate_t = TransformComponent(aiCar.getVehicleRigidBody());
	std::string nameplate_name = "textures/nameplates/" + aiCar.m_name + ".png";
	VFXBillboard nameplate_b = VFXBillboard(nameplate_name, glm::vec3(1, 1, 0));
	float size = 0.65f;
	nameplate_t.setScale(glm::vec3(size * 3.f, size, 0));
	nameplate_t.setPosition(glm::vec3(0, 2.6, 1));

	scene.AddComponent(nameplate.guid, nameplate_t);
	scene.AddComponent(nameplate.guid, nameplate_b);

	TransformComponent& wrongWaySign_t = scene.GetComponent<TransformComponent>(nameplate.guid);

  return aiDriver_e.guid; 

}



std::vector<glm::vec3> spawnpointsAlongAxis(int rows, int cols,float spread, glm::vec3 axis, glm::vec3 start)
{
	std::vector<glm::vec3> result;

	axis = glm::normalize(axis);
	const glm::vec3 UP = {0.f, 1.f,0.f};
	// spawn the cars along the axis
	glm::vec3 binormal = glm::cross(axis,UP);
	// spawn a row of cars on binormal of axis

	float width = static_cast<float>(rows) * spread;
	float height = static_cast<float>(cols) * spread; 


	glm::vec3 offset = binormal * ((width / 2) - (spread / 2) );

	for (int col =0; col < cols; col++)
	{
		glm::vec3 colStart = offset + start + (-axis * (col * spread));
		for (int row = 0; row < rows; row++)
		{
			glm::vec3 spawnPosition = colStart + (-binormal * (row * spread));
			result.push_back(spawnPosition);
		}
	}

	return result;

}


std::vector<std::string> getPlayerNames()
{
	std::vector<std::string> names;

	names.push_back("Zeus");
	names.push_back("Poseidon");
	names.push_back("Ares");
	names.push_back("Aphrodite");
	names.push_back("Hera");
	names.push_back("Demeter");
	names.push_back("Athena");
	names.push_back("Apollo");
	names.push_back("Artemis");
	names.push_back("Hephaestus");
	names.push_back("Hermes");
	names.push_back("Dionysus");

	auto rng = std::default_random_engine {};
	std::shuffle(std::begin(names), std::end(names), rng);
	return names;
}