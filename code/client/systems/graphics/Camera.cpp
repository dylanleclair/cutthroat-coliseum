#include "../GraphicsSystem.h"
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "core/ecs.h"
#include "../components.h"

#include "glm/gtc/quaternion.hpp"
#include <glm/gtx/quaternion.hpp>



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
	cameraPos = _position;
}

void Camera::update(TransformComponent& _carTransform)
{
	/*
	* calculate the camera position
	*/
	//calculate where the camera should aim to be positioned
	glm::vec3 cameraTargetLocation = glm::translate(glm::mat4(1), _carTransform.getTranslation()) * toMat4(_carTransform.getRotation()) * glm::vec4(GraphicsSystem::follow_cam_x, GraphicsSystem::follow_cam_y, GraphicsSystem::follow_cam_z, 1);
	//calculate the speed of the car
	float speed = glm::distance(previousCarPosition, _carTransform.getTranslation());
	//calculate the vector from the cameras current position to the target position
	glm::vec3 cameraOffset = getPos() - cameraTargetLocation;
	if (glm::length(cameraOffset) != 0) {
		float followDistance = (speed / (speed + GraphicsSystem::follow_correction_strength)) * GraphicsSystem::maximum_follow_distance;
		cameraOffset = glm::normalize(cameraOffset) * followDistance;
		setPos(cameraTargetLocation + cameraOffset);
	}
	previousCarPosition = _carTransform.getTranslation();
}
