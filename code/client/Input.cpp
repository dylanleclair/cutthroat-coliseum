#include "Input.h"

namespace ControllerInput {
	SDL_GameController* controller = nullptr;

	SDL_GameController* controllers[4]; 

	// NOTE: only call when no controller is plugged in!
	void init_controller() {
		controller = SDL_GameControllerOpen(0);
	}

	// NOTE: only call when a controller is plugged in!
	void deinit_controller() {
		SDL_GameControllerClose(controller);
		controller = nullptr;
	}

	
	void initControllers() 
	{
		
		int numcontrollers = SDL_NumJoysticks();
		for (int i = 0; i < numcontrollers && i < 4; i++)
		{
			if (controllers[i] == nullptr)
				controllers[i] = SDL_GameControllerOpen(i);
		}
		
	}

	void destroyControllers()
	{
		for (int i = 0; i < 4; i++)
		{
			
			SDL_GameController* c = controllers[i];
			if (c != nullptr)
			{
				SDL_GameControllerClose(c);
				controller = nullptr;
			}
		}
	}

	int getNumberPlayers()
	{
		int count{0};
		// returns the number of controllers non null
		for (int i=0; i < 4; i++)
		{
			if (controllers[i] != nullptr)  count++;
		}
		return count;
	}

}