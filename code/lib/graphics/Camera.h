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
	glm::vec3 getPos();
private:
	glm::vec3 cameraPos = glm::vec3(0.0f, 2.5f, 1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f);
	float fspeed = 0, hspeed = 0, panHorizontal = -90.0f, panVertical = 0.0f, lastX = 0, lastY = 0;
	bool leftMouseButtonPressed = false, firstMouse = true;
	const float cameraSpeed = 0.1;
};

