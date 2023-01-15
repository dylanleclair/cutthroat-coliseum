#include "Window.h"

#include "Log.h"

#include <iostream>

void sdl_error_handler(int sdl_return) {
	if (!sdl_return) return;
	Log::error("SDL error: ", SDL_GetError());
	SDL_Quit();
	exit(1);
}


Window::Window(int width, int height, const char* title)
	: window(nullptr)
{
	// specify OpenGL version
	// XXX(beau): handle errors?
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

	window = std::unique_ptr<SDL_Window, WindowDeleter>(SDL_CreateWindow("CPSC 453", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL));
	if (!window)
		sdl_error_handler(1);

	// TODO:(beau) clean up
	SDL_GLContext context = SDL_GL_CreateContext(window.get());
	if (!context)
		sdl_error_handler(1);

	// vsync
	sdl_error_handler(SDL_GL_SetSwapInterval(1));

	// initialize OpenGL extensions for the current context (this window)
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		Log::error("WINDOW glewInit error:{}", glewGetErrorString(err));
		throw std::runtime_error("Failed to initialize GLEW");
	}

	// TODO(beau): put this somewhere obvious
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(window.get(), context);

	// VOLATILE: must match version specified in shaders!
	ImGui_ImplOpenGL3_Init("#version 460 core");
}


glm::ivec2 Window::getPos() const {
	int x, y;
	SDL_GetWindowPosition(window.get(), &x, &y);
	return glm::ivec2(x, y);
}


glm::ivec2 Window::getSize() const {
	int w, h;
	SDL_GetWindowSize(window.get(), &w, &h);
	return glm::ivec2(w, h);
}
