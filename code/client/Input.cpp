#include "Input.h"

namespace ControllerInput {
	SDL_GameController* controller = nullptr;

	// NOTE: only call when no controller is plugged in!
	void init_controller() {
		controller = SDL_GameControllerOpen(0);
	}

	// NOTE: only call when a controller is plugged in!
	void deinit_controller() {
		SDL_GameControllerClose(controller);
		controller = nullptr;
	}
}