#pragma once

#include "SDL.h"

namespace ControllerInput {
	extern SDL_GameController* controller;

	// NOTE: only call when no controller is plugged in!
	void init_controller();

	// NOTE: only call when a controller is plugged in!
	void deinit_controller();
}