#define TOML_EXCEPTIONS 0 // NOTE(beau): I don't want exceptions lmao
#include "toml++/toml.h"

#define FILEPATH "configs/CarPhysics.toml"
#define ASSETS_FILEPATH "../../code/assets/configs/CarPhysics.toml"

struct CarPhysics {
	float m_suspension_force;
	float m_acceleration;
};

// NOTE(beau): imo this could inherit from some interface - beau
struct CarPhysicsConfig {
	CarPhysics tempdata; // to be copied into all cars via ecs system
	toml::table table;

	CarPhysicsConfig(bool serializing);

	void serialize();
	void deserialize();
};
