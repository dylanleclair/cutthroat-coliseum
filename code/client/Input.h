#pragma once

#include "SDL.h"

namespace ControllerInput {
	extern SDL_GameController* controller;
	extern SDL_GameController* controllers[4]; 

	// NOTE: only call when no controller is plugged in!
	void init_controller();

	// NOTE: only call when a controller is plugged in!
	void deinit_controller();

	void initControllers();

	void destroyControllers();

	int getNumberPlayers();

}