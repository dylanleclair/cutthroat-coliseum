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
	float m_acceleration;
	float m_suspension_force;
};

// NOTE(beau): could this inherit from some interface? - beau
struct CarPhysicsSerde {
	CarPhysics &m_car_config;
	toml::table table;

	CarPhysicsSerde(CarPhysics &carPhysics);

	void serialize();
	void deserialize();
};
