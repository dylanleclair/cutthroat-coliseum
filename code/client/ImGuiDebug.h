#include "CarSample.h";
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include <glm/glm.hpp>

using namespace physx;

// Rigid Body Params
static float rigid_mass;
static PxVec3 rigid_MOI;

// Brake Command Response Params
static float brake_max;
static PxReal *wheel_response_multip;

void variableInit();
void vehicleTuning();
void reloadVehicleJSON();