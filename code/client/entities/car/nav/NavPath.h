#pragma once
#include "glm/glm.hpp"
#include "../../../curve/Curve.h"

struct CheckPoint {
    CheckPoint(int index, glm::vec3 pos) : index(index), position(pos) {}
    int index;
    glm::vec3 position;
};

struct NavPath {

    // api to pass in the current path 
    NavPath(Curve* checkpoints) : checkpoints(checkpoints), currentCheckpoint(CheckPoint{0,checkpoints->start()}) {}

    glm::vec3 getDirectionVector(glm::vec3 currentPos);
    glm::vec3 getNextPoint(glm::vec3 currentPos, bool& outDidLap);

    void resetNav();

    CheckPoint currentCheckpoint;
    Curve* checkpoints;
};