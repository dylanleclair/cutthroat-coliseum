#include "CarSample.h";
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include <glm/glm.hpp>

using namespace glm;

static float rigid_mass;
static physx::PxVec3 rigid_MOI;

void variableInit();
void vehicleTuning();
void reloadVehicleJSON();