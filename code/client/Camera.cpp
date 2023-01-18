#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera() {}

// takes in a point and calculates the angles for the camera to look at that point from its position
void Camera::lookat(float x, float y, float z) {
}

glm::mat4 Camera::getView()
{
	glm::vec3 cameraFront = callbacks->cameraDirection;
	
	cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * callbacks->hspeed;
	cameraPos += cameraSpeed * cameraFront * callbacks->fspeed;

	return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

/*
* Used to forward callback events into the camera
*/
void Camera::input(SDL_Event& _event) {
	//THESE ARE TEMPOARY!!!!
	static bool leftMouseButtonPressed, firstMouse = false;
	static float panup, pandown, fspeed, hspeed, panHorizontal, panVertical, lastX, lastY = 0;

	if(_event.type == SDL_KEYDOWN)
	{
		if (_event.key.keysym.sym == SDLK_w)
			fspeed = 1.0f;
		if (_event.key.keysym.sym == SDLK_s)
			fspeed = -1.0f;
		if (_event.key.keysym.sym == SDLK_a)
			hspeed = -1.0f;
		if (_event.key.keysym.sym == SDLK_d)
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
			clickX = mouseX;
			clickY = mouseY;
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
		float xpos = _event.motion.x;
		float ypos = _event.motion.y;
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
