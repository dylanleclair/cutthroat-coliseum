#pragma once
#include "Car.h"
#include "systems/ai.h"

struct AICar : Car {


    // TODO(dylan): add a renderer to render the path the AI car is following

    AICar() : Car() {}
        // all the physics stuff lives in the physics system
    // AICar(physics::PhysicsSystem* physicsSystem, PathfindingComponent& pathfinder) :  Car()
    // {
    //     // The vehicle with engine 
    //     bool success = initVehicle();
    //     if (!success)
    //     {
    //         std::cerr << "error initializing vehicle!" << std::endl;
    //     }
    // }
    virtual void Update(Guid carGuid, ecs::Scene& scene, float deltaTime);

};