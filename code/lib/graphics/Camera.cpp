#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Camera::Camera() {
	glm::vec3 direction;
	direction.x = cos(glm::radians(panHorizontal)) * cos(glm::radians(panVertical));
	direction.y = sin(glm::radians(panVertical));
	direction.z = sin(glm::radians(panHorizontal)) * cos(glm::radians(panVertical));
	cameraDirection = glm::normalize(direction);
}

glm::mat4 Camera::getView()
{
	glm::vec3 cameraFront = cameraDirection;
	
	cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * hspeed;
	cameraPos += cameraSpeed * cameraFront * fspeed;

	return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

/*
* Used to forward callback events into the camera
*/
void Camera::input(const SDL_Event& _event) {
	auto& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard && _event.type == SDL_MOUSEBUTTONDOWN) return;

	if(_event.type == SDL_KEYDOWN)
	{
		if (_event.key.keysym.sym == SDLK_UP)
			fspeed = 1.0f;
		if (_event.key.keysym.sym == SDLK_DOWN)
			fspeed = -1.0f;
		if (_event.key.keysym.sym == SDLK_LEFT)
			hspeed = -1.0f;
		if (_event.key.keysym.sym == SDLK_RIGHT)
			hspeed = 1.0f;
	}
	if (_event.type == SDL_KEYUP)
	{
		fspeed = 0.0f;
		hspeed = 0.0f;
	}

	if (_event.type == SDL_MOUSEBUTTONDOWN)
	{
		if (_event.button.button == SDL_BUTTON_LEFT)
		{
			leftMouseButtonPressed = true;
		}
	}
	else if (_event.type == SDL_MOUSEBUTTONUP)
	{
		leftMouseButtonPressed = false;
		firstMouse = true;
	}

	//move the camera
	if (leftMouseButtonPressed)
	{
		
		int xpos, ypos;
		SDL_GetMouseState(&xpos, &ypos);
		//The following code was adapted from https://learnopengl.com/Getting-started/Camera
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = (xpos - lastX) * 0.1;
		float yoffset = (lastY - ypos) * 0.1;
		lastX = xpos;
		lastY = ypos;

		panHorizontal += xoffset;
		panVertical += yoffset;

		glm::vec3 direction;
		direction.x = cos(glm::radians(panHorizontal)) * cos(glm::radians(panVertical));
		direction.y = sin(glm::radians(panVertical));
		direction.z = sin(glm::radians(panHorizontal)) * cos(glm::radians(panVertical));
		cameraDirection = glm::normalize(direction);
	}
}

glm::vec3 Camera::getPos()
{
	return cameraPos;
}

void Camera::setPos(glm::vec3 _position)
{
	velocity = _position - cameraPos;
	cameraPos = _position;
}
