#define TOML_EXCEPTIONS 0 // NOTE(beau): I don't want exceptions lmao
#include "toml++/toml.h"

#define FILEPATH "configs/CarPhysics.toml"

// assets filepath based on which os
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#define ASSETS_FILEPATH
"../../../../assets/configs/CarPhysics.toml"
#else
#define ASSETS_FILEPATH "../../code/assets/configs/CarPhysics.toml"
#endif

struct CarPhysics {
	float m_suspension_force;
	float m_acceleration;
};

// NOTE(beau): imo this could inherit from some interface - beau
struct CarPhysicsConfig {
	toml::table table;

	CarPhysicsConfig(bool serializing);

	void serialize();
	void deserialize();
};
