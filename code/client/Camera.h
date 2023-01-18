#pragma once
#include "SDL.h"
#include "SDL_main.h"
#include "SDL_opengl.h"
#include <glm/glm.hpp>

class Camera {
public:
	Camera();
	void lookat(float, float, float);
	glm::mat4 getView();
	void input(const SDL_Event&);
private:
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.5f, 1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f);
	float panup, pandown, fspeed, hspeed, panHorizontal, panVertical, lastX, lastY = 0;
	bool leftMouseButtonPressed, firstMouse = false;
	const float cameraSpeed = 0.5;
};

