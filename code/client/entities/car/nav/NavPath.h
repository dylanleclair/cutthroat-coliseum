#pragma once
#include <vector>
#include "glm/glm.hpp"

struct CheckPoint {
    CheckPoint(int index, glm::vec3 pos) : index(index), position(pos) {}
    int index;
    glm::vec3 position;
};

struct NavPath {

    // api to pass in the current path 
    NavPath(std::vector<glm::vec3> checkpoints) : checkpoints(checkpoints), currentCheckpoint(CheckPoint{0,checkpoints[0]}) {}

    glm::vec3 getDirectionVector(glm::vec3 currentPos);
    glm::vec3 getNextPoint(glm::vec3 currentPos, bool& outDidLap);

    void resetNav();

    CheckPoint currentCheckpoint;
    std::vector<glm::vec3> checkpoints;

};