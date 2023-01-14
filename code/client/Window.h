#pragma once

//------------------------------------------------------------------------------
// This file contains classes that provide a simpler and safer interface for
// interacting with a GLFW window following RAII principles
//------------------------------------------------------------------------------

#include <GL/glew.h>
#include <SDL_render.h>
#include <glm/glm.hpp>

#include "SDL.h"
#include "SDL_main.h"
#include "SDL_opengl.h"

#include <memory>


// TODO(beau): decide if we want to use callbacks for input handling, or
//             or just handle input events in main. If the latter, scrap
//             all this callback stuff
// Class that specifies the interface for the most common GLFW callbacks
//
// These are the default implementations. You can write your own class that
// extends this one and overrides the implementations with your own
class CallbackInterface {
public:
	virtual void keyCallback(int key, int scancode, int action, int mods) {}
	virtual void mouseButtonCallback(int button, int action, int mods) {}
	virtual void cursorPosCallback(double xpos, double ypos) {}
	virtual void scrollCallback(double xoffset, double yoffset) {}
	virtual void windowSizeCallback(int width, int height) { glViewport(0, 0, width, height); }
};


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
class Window {

public:
	// TODO(beau): decide if we want to use callbacks for input handling, or
	//             or just handle input events in main. If the latter, scrap
	//             all this callback stuff
	Window(
		std::shared_ptr<CallbackInterface> callbacks, int width, int height, const char* title
	);
	Window(int width, int height, const char* title);




	// TODO(beau): decide if we want to use callbacks for input handling, or
	//             or just handle input events in main. If the latter, scrap
	//             all this callback stuff
	void setCallbacks(std::shared_ptr<CallbackInterface> callbacks);

	glm::ivec2 getPos() const;
	glm::ivec2 getSize() const;

	int getX() const { return getPos().x; }
	int getY() const { return getPos().y; }
	
	int getWidth() const { return getSize().x; }
	int getHeight() const { return getSize().y; }

	// NOTE(beau): leave this in in case we need to change contexts
	// void makeContextCurrent() { glfwMakeContextCurrent(window.get()); }

	void swapBuffers() { SDL_GL_SwapWindow(window.get()); }


	// NOTE(beau): I *think* this should be here so we can do the callback thing like before,
	// BUT if we don't have to do that why not just switch on the sdl event in the render loop
	SDL_Event event;

private:
	std::unique_ptr<SDL_Window, WindowDeleter> window; // owning ptr (from GLFW)
	// TODO(beau): decide if we want to use callbacks for input handling, or
	//             or just handle input events in main. If the latter, scrap
	//             all this callback stuff
	std::shared_ptr<CallbackInterface> callbacks;      // optional shared owning ptr (user provided)

	// void connectCallbacks();

	// static void defaultWindowSizeCallback(SDL_Window* window, int width, int height) { glViewport(0, 0, width, height); }

	// TODO(beau): decide if we want to use callbacks for input handling, or
	//             or just handle input events in main. If the latter, scrap
	//             all this callback stuff
	// Meta callback functions. These bind to the actual glfw callback,
	// get the actual callback method from user data, and then call that.
	// static void keyMetaCallback(SDL_Window* window, int key, int scancode, int action, int mods);
	// static void mouseButtonMetaCallback(SDL_Window* window, int button, int action, int mods);
	// static void cursorPosMetaCallback(SDL_Window* window, double xpos, double ypos);
	// static void scrollMetaCallback(SDL_Window* window, double xoffset, double yoffset);
	// static void windowSizeMetaCallback(SDL_Window* window, int width, int height);
};

