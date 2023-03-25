
#include "AICar.h"
#include "core/pathfinding.h"
#include "../../systems/GraphicsSystem.h"
#include "../../utils/PxConversionUtils.h"
#include "glm/glm.hpp"


NavPath generateCirclePath(int radius) {

    // generate a circle
    std::vector<glm::vec3> circle;

    float step{0.4f};
    for (float i =0; i < 2 * 3.14; i += step)
    {
        float x = std::cos(i);
        float z = std::sin(i);
        circle.push_back({radius * x, 0.f, radius * z});
    }

    return NavPath{circle};

}

void AICar::Initialize(NavPath* pathToFollow)
{
    m_navPath = pathToFollow;
}

glm::vec3 roundPositionToGraph(glm::vec3 pos)
{
    return glm::vec3(floor(pos.x), 0.f, floor(pos.z));
}

float euclideanBasic(glm::vec3 a, glm::vec3 b)
{
    float deltaX = b.x - a.x;
    float deltaZ = b.z - a.z;

    float dist = pow(deltaX, 2) + pow(deltaZ, 2);
    return sqrt(dist);
}




Command AICar::pathfind(glm::vec3 currentPosition)
{

    assert(m_navPath != nullptr);

    Command command = {0.f, 0.f, 0.f, m_TargetGearCommand};

    bool didLap{false};

    glm::vec3 targetPos = m_navPath->getNextPoint(currentPosition,didLap);
    // find rotation matrix of car
    PxTransform carPose = this->m_Vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose();

    // find the direction vector of the vehicle
    glm::quat vehicleQuat = PxtoGLM(carPose.q);
    glm::mat4 vehicleRotM = glm::toMat4(vehicleQuat);
    glm::vec3 headingDir = glm::vec3{vehicleRotM * glm::vec4{0.f, 0.f, -1.f, 1.f}};
    
    // target direction vector
    PxVec3 targetDir = GLMtoPx(targetPos) - carPose.p;
    
    // only drive to the target if it's far enough away (for now)


    // need to scale down throttle based on the angle we're trying to turn
    // this will help the AI turn!

    command.throttle = 1.f;

    targetDir.normalize();

    // radians!!

    float angleBetween = targetDir.dot(GLMtoPx(headingDir));
    float actualAngle = acos(angleBetween);

    std::cout << "actual angle " << actualAngle << std::endl;
    std::cout << "angle between " << angleBetween << std::endl;

    // if almost parallel, don't worry about steering
    if (abs(actualAngle) < 0.10f)
    {
        command.steer = 0.0f;
    } else {
        
        PxVec3 cross = GLMtoPx(headingDir).cross(targetDir);
        if (cross.y < 0)
        {
            command.steer = 1.0f;
        } else {
            command.steer = -1.0f;
        }
    }

    checkFlipped(carPose);


    // CHECK IF GOING TO RUN INTO AN OBSTACLE
    // SEE IF OBSTACLE UP AHEAD

    // INSTEAD OF CASTING FROM VEHICLE
    // FOR EACH VERTEX, SEE IF OBSTACLE IN UP DIRECTION
    // 

    return command;
}