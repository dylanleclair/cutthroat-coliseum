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

	
	bool controllerAlreadyRegistered(SDL_GameController* gc)
	{
		for (int i = 0; i < 4; i++)
		{
			if (controllers[i] == gc)
			{
				return true;
			}
		}
		return false;	
	}

	void initControllers() 
	{
		
		

		int numcontrollers = SDL_NumJoysticks();
		
		for (int i = 0; i < numcontrollers && i < 4; i++)
		{
			if (controllers[i] == nullptr)
			{
				SDL_GameController* c = SDL_GameControllerOpen(i);
				if (!controllerAlreadyRegistered(c))
				{
					controllers[i] = c;
				}
			}
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
				c = nullptr;
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