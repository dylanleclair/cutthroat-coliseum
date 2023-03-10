#pragma once
#include "SDL.h"
#include "SDL_main.h"
#include "SDL_opengl.h"
#include <glm/glm.hpp>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

class Camera {
public:
	Camera();
	void lookat(glm::vec3 _target);
	glm::mat4 getView();
	void input(const SDL_Event&);
	glm::vec3 getPos();
	void setPos(glm::vec3 _position);
private:
	friend class GraphicsSystem;
	glm::vec3 cameraPos = glm::vec3(35.0f, 6.0f, -15.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f); //DO not set, it is overwritten on construction!
	float fspeed = 0, hspeed = 0, panHorizontal = 90.0f, panVertical = -10.0f, lastX = 0, lastY = 0;
	bool leftMouseButtonPressed = false, firstMouse = true;
	const float cameraSpeed = 0.1;

	glm::vec3 velocity = glm::vec3(0);
};

