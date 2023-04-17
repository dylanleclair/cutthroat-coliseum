// TODO(beau): sdl cleanup:
//             There are a number of sdl things we could clean up, we simply need to figure out when/where
//             - SDL_Quit()
//             - clean up OpenGL context (will we switch contexts?)
//             - clean up the window?
//             - other things iirc
//             IMO these things will become more clear once we know more about the architecture of our program.
//             For now, we can leave these things as we're not leaking memory and the OS is cleaning up after us.
//             Note that the glfw code also didn't do a great job of cleaning up after itself
#include "Window.h"

#include <iostream>

void sdl_error_handler(int sdl_return) {
	if (!sdl_return) return;
	printf("SDL error: ", SDL_GetError());
	SDL_Quit();
	exit(1);
}

void FAIL() {
	std::cout << "failed to make window\n";
	exit(1);
}

Window::Window(int width, int height, const char* title)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		FAIL();

#if defined RMLUI_PLATFORM_EMSCRIPTEN
	// GLES 3.0 (WebGL 2.0)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
	// GL 3.3 Core
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

	// Request stencil buffer of at least 8-bit size to supporting clipping on transformed elements.
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// Enable linear filtering and MSAA for better-looking visuals, especially when transforms are applied.
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

	const Uint32 window_flags = (SDL_WINDOW_OPENGL);

	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, window_flags);
	if (!window)
	{
		// Try again on low-quality settings.
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, window_flags);
		if (!window)
		{
			fprintf(stderr, "SDL error on create window: %s\n", SDL_GetError());
			FAIL();
		}
	}

	glcontext = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, glcontext);
	SDL_GL_SetSwapInterval(1);

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		printf("WINDOW glewInit error:{}", glewGetErrorString(err));
		throw std::runtime_error("Failed to initialize GLEW");
	}
	
#define ImGUI_Enabled
#ifdef ImGUI_Enabled
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(window, glcontext);
	// VOLATILE: must match version specified in shaders!
	ImGui_ImplOpenGL3_Init("#version 410 core");
#endif

	std::cout << "finished initalizing window\n\n";
}


glm::ivec2 Window::getPos() const {
	int x, y = 0;
	//SDL_GetWindowPosition(window.get(), &x, &y);
	return glm::ivec2(x, y);
}


glm::ivec2 Window::getSize() const {
	int w, h = 0;
	//SDL_GetWindowSize(window.get(), &w, &h);
	return glm::ivec2(w, h);
}

void Window::RenderAndSwap()
{
	glDisable(GL_BLEND);
#define ImGUI_Enabled
#ifdef ImGUI_Enabled
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glDisable(GL_FRAMEBUFFER_SRGB);
	ImGui::Render();
	glViewport(0, 0, 1200, 800);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glEnable(GL_FRAMEBUFFER_SRGB);
#endif
	//swap the buffers
	SDL_GL_SwapWindow(window);
}