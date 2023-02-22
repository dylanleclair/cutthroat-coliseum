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
static PxReal *brake_response_multip;

// Steer Command Response Params
static PxF32 steer_max_response;
static PxReal *steer_multiplier;

// Ackerman Angle

static PxU32 *ack_wheel_ids;
static PxReal ack_wheel_base;
static PxReal ack_track_width;
static PxReal ack_strength;

// Wheel Params
static int number_of_wheels; // This is a hardcoded value, may need to grab this dynamically
static PxReal wheel_radius[4];
static PxReal wheel_half_width[4];
static PxReal wheel_mass[4];
static PxReal wheel_moi[4];
static PxReal wheel_dampening[4];

void variableInit();
void vehicleTuning();
void reloadVehicleJSON();