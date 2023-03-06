#pragma once
#include "Car.h"
#include "systems/ai.h"
#include "systems/ecs.h"
#include <vector>
#include "glm/glm.hpp"


struct NavPath;

NavPath generateCirclePath(int radius);

struct CheckPoint {
    CheckPoint(int index, glm::vec3 pos) : index(index), position(pos) {}
    int index;
    glm::vec3 position;
};

struct NavPath {

    // api to pass in the current path 
    NavPath(std::vector<glm::vec3> checkpoints) : checkpoints(checkpoints), currentCheckpoint(CheckPoint{0,checkpoints[0]}) {}

    glm::vec3 getNextPoint(glm::vec3 currentPos, bool& outDidLap) {

        // check if current position is closer to the next checkpoint than the previous
        // make sure to wraparound on last checkpoint !
        int nextIndex = (currentCheckpoint.index == checkpoints.size() -1 ) ? 0 : currentCheckpoint.index + 1;

        CheckPoint nextCP = CheckPoint{nextIndex, checkpoints[nextIndex]};

        float distanceToCurrent = glm::distance(currentCheckpoint.position, currentPos);
        float distanceToNext = glm::distance(nextCP.position, currentPos);



        if (std::min(distanceToCurrent, distanceToNext) == distanceToNext)
        {
            // if the smaller distance is to the next checkpoint, move to next checkpoint!
            currentCheckpoint = nextCP;

            outDidLap = (currentCheckpoint.index == checkpoints.size() -1) ? true : false;
            
            nextIndex = (currentCheckpoint.index == checkpoints.size() -1 ) ? 0 : currentCheckpoint.index + 1;

            nextCP = CheckPoint{nextIndex, checkpoints[nextIndex]};
            distanceToCurrent = glm::distance(currentCheckpoint.position, currentPos);
            distanceToNext = glm::distance(nextCP.position, currentPos);
            // std::cout << "dist to next: " << distanceToNext << std::endl;
            // std::cout << "dist to current: " << distanceToNext << std::endl;
        }
        return nextCP.position;
    }

    CheckPoint currentCheckpoint;
    std::vector<glm::vec3> checkpoints;

};

struct AICar : Car {

    // TODO(dylan): add a renderer to render the path the AI car is following

    NavPath* m_navPath;
    int m_lapCount;

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

    void Initialize(NavPath* pathToFollow); 

    Command pathfind(glm::vec3 currentPosition);

    Command pathfind(glm::vec3 currentPosition, ecs::Scene& scene, Guid targetEntity);


};