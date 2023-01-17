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
	void input(SDL_Event&);
	float x, y;
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f);
};

