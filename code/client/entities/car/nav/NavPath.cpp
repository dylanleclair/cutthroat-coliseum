#include "NavPath.h"

    glm::vec3 NavPath::getDirectionVector(glm::vec3 currentPos){
        return checkpoints->forward(currentPos);
    };
    
    glm::vec3 NavPath::getNextPoint(glm::vec3 currentPos, bool& outDidLap) {

        // check if current position is closer to the next checkpoint than the previous
        // make sure to wraparound on last checkpoint !
        int nextIndex = checkpoints->nextIndex(currentPos);

        CheckPoint nextCP = CheckPoint{nextIndex, (*checkpoints)[nextIndex]};

        float distanceToCurrent = glm::distance(currentCheckpoint.position, currentPos);
        float distanceToNext = glm::distance(nextCP.position, currentPos);

        if (std::min(distanceToCurrent, distanceToNext) == distanceToNext)
        {
            // if the smaller distance is to the next checkpoint, move to next checkpoint!
            currentCheckpoint = nextCP;

            outDidLap = (currentCheckpoint.index == checkpoints->size() -1) ? true : false;
            
            nextIndex = (currentCheckpoint.index + 1 >= checkpoints->size()) ? 0 : currentCheckpoint.index + 1;

            nextCP = CheckPoint{nextIndex, (*checkpoints)[nextIndex]};
            distanceToCurrent = glm::distance(currentCheckpoint.position, currentPos);
            distanceToNext = glm::distance(nextCP.position, currentPos);
        }
        return nextCP.position;
    }

    void NavPath::resetNav()
    {
        currentCheckpoint = CheckPoint{0,(*checkpoints)[0]};
    }