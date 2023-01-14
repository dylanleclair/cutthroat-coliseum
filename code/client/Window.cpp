#include "Window.h"

#include "Log.h"

#include <iostream>

void sdl_error_handler(int sdl_return) {
	if (!sdl_return) return;
	Log::error("SDL error: ", SDL_GetError());
	SDL_Quit();
	exit(1);
}


// ---------------------------
// static function definitions
// ---------------------------

void Window::keyMetaCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	callbacks->keyCallback(key, scancode, action, mods);
}


void Window::mouseButtonMetaCallback(GLFWwindow* window, int button, int action, int mods) {
	CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	callbacks->mouseButtonCallback(button, action, mods);
}


void Window::cursorPosMetaCallback(GLFWwindow* window, double xpos, double ypos) {
	CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	callbacks->cursorPosCallback(xpos, ypos);
}


void Window::scrollMetaCallback(GLFWwindow* window, double xoffset, double yoffset) {
	CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	callbacks->scrollCallback(xoffset, yoffset);
}


void Window::windowSizeMetaCallback(GLFWwindow* window, int width, int height) {
	CallbackInterface* callbacks = static_cast<CallbackInterface*>(glfwGetWindowUserPointer(window));
	callbacks->windowSizeCallback(width, height);
}


// ----------------------
// non-static definitions
// ----------------------

Window::Window(
	std::shared_ptr<CallbackInterface> callbacks, int width, int height,
	const char* title, GLFWmonitor* monitor, GLFWwindow* share
)
	: window(nullptr)
	, callbacks(callbacks)
{
	// specify OpenGL version
	// XXX(beau): handle errors?
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

	window = std::unique_ptr<SDL_Window, WindowDeleter>(SDL_CreateWindow("CPSC 453", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL));
	if (!window)
		sdl_error_handler(1);
	// create window
	// window = std::unique_ptr<GLFWwindow, WindowDeleter>(glfwCreateWindow(width, height, title, monitor, share));
	//
	// if (window == nullptr) {
	// 	Log::error("WINDOW failed to create GLFW window");
	// 	throw std::runtime_error("Failed to create GLFW window.");
	// }
	// glfwMakeContextCurrent(window.get());

	// TODO:(beau) clean up
	if (!SDL_GL_CreateContext(window.get()))
		sdl_error_handler(1);

	// vsync
	sdl_error_handler(SDL_GL_SetSwapInterval(1));

	// initialize OpenGL extensions for the current context (this window)
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		Log::error("WINDOW glewInit error:{}", glewGetErrorString(err));
		throw std::runtime_error("Failed to initialize GLEW");
	}

	// glfwSetWindowSizeCallback(window.get(), defaultWindowSizeCallback);

	// if (callbacks != nullptr) {
	// 	connectCallbacks();
	// }
}


Window::Window(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share)
	: Window(nullptr, width, height, title, monitor, share)
{}


void Window::connectCallbacks() {
	// set userdata of window to point to the object that carries out the callbacks
	// glfwSetWindowUserPointer(window.get(), callbacks.get());

	// bind meta callbacks to actual callbacks
	// glfwSetKeyCallback(window.get(), keyMetaCallback);
	// glfwSetMouseButtonCallback(window.get(), mouseButtonMetaCallback);
	// glfwSetCursorPosCallback(window.get(), cursorPosMetaCallback);
	// glfwSetScrollCallback(window.get(), scrollMetaCallback);
	// glfwSetWindowSizeCallback(window.get(), windowSizeMetaCallback);
}


void Window::setCallbacks(std::shared_ptr<CallbackInterface> callbacks_) {
	callbacks = callbacks_;
	connectCallbacks();
}


glm::ivec2 Window::getPos() const {
	int x, y;
	// glfwGetWindowPos(window.get(), &x, &y);
	return glm::ivec2(x, y);
}


glm::ivec2 Window::getSize() const {
	int w, h;
	// glfwGetWindowSize(window.get(), &w, &h);
	return glm::ivec2(w, h);
}
