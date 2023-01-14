#pragma once

//------------------------------------------------------------------------------
// This file contains classes that provide a simpler and safer interface for
// interacting with a SDL window following RAII principles
//------------------------------------------------------------------------------

#include <GL/glew.h>
#include <SDL_render.h>
#include <glm/glm.hpp>

#include "SDL.h"
#include "SDL_main.h"
#include "SDL_opengl.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "imgui_impl_opengl3.h"

#include <memory>



// Functor for deleting a SDL window.
//
// This is used as a custom deleter with std::unique_ptr so that the window
// is properly destroyed when std::unique_ptr needs to clean up its resource
struct WindowDeleter {
	void operator() (SDL_Window* window) const {
		SDL_DestroyWindow(window);
	}
};


// Main class for creating and interacting with a SDL window.
// Only wraps the most fundamental parts of the API
struct Window {
	Window(int width, int height, const char* title);

	glm::ivec2 getPos() const;
	glm::ivec2 getSize() const;

	int getX() const { return getPos().x; }
	int getY() const { return getPos().y; }
	
	int getWidth() const { return getSize().x; }
	int getHeight() const { return getSize().y; }

	// NOTE(beau): leave this in in case we need to change contexts, but refactor to sdl2
	// void makeContextCurrent() { glfwMakeContextCurrent(window.get()); }

	void swapBuffers() { SDL_GL_SwapWindow(window.get()); }

	SDL_Event event;

	std::unique_ptr<SDL_Window, WindowDeleter> window; // owning ptr (from SDL)
};
