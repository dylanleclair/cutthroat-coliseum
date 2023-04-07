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
#define ImGUI_Enabled
#ifdef ImGUI_Enabled
	auto& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard && _event.type == SDL_MOUSEBUTTONDOWN) return;
#endif
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

void Camera::update(TransformComponent& _carTransform, bool isReversing, glm::vec3 carVelocity, float dt)
{
	/*
	* calculate the camera position
	*/
	//calculate where the camera should aim to be positioned
	glm::vec3 cameraTargetLocation = glm::translate(glm::mat4(1), _carTransform.getTranslation()) * toMat4(_carTransform.getRotation()) * glm::vec4(GraphicsSystem::follow_cam_x, GraphicsSystem::follow_cam_y, GraphicsSystem::follow_cam_z, 1);
	//calculate the speed of the car
	
	if (!initalized) {
		initalized = true;
		previousCarPosition = _carTransform.getTranslation();
	}

	previousCarPosition = _carTransform.getTranslation();

	float FOVBounceMax = 50;
	float maxFOVstable = 45;
	float minFOV = 30;
	static char state = 0; //0 = no change, 1 = increasing to FOVBounceMax, 2 = decreasing from FOVBounceMax, 3 = stable at maxFOV
	float carSpeed = glm::length(carVelocity);

	//determine the camera state
	if (carSpeed >= 0.01) {
		if (state == 0 && carSpeed > 20)
			state = 1;
		else if (state == 1 && FOV >= FOVBounceMax)
			state = 2;
		else if (state == 2 && FOV <= maxFOVstable)
			state = 3;
		else if (carSpeed <= 10)
			state = 0;
	}

	if (isReversing)
		state = 0;

	//decrease it
	if (state == 3 || state == 0)
		FOV -= 20 * dt;
	else if (state == 2)
		FOV -= 5 * dt;


	if(state == 1 || state == 3)
		FOV += carSpeed * dt;

	//bound the FOV
	if(state == 3)
		FOV = fmin(maxFOVstable, FOV);
	FOV = fmax(minFOV, FOV);
	
	//std::cout << "speed: " << carSpeed << "  FOV: " << FOV << " State: " << (int)state << '\n';
	FOV = 45;

	glm::vec3 currentCamLocation = getPos();
	
	//update the camera using a spring 
	const float k = 3;
	const float c = 1;
	const float r = 0.5;
	const float cameraMass = 1;
	const glm::vec3 s = cameraTargetLocation - getPos();
	glm::vec3 Fs = (k * (glm::length(s) - r)) * glm::normalize(s);
	glm::vec3 Fd = ((glm::dot(-c * (carVelocity - cameraVelocity), s)) / glm::length(s)) * glm::normalize(s);

	glm::vec3 a = (Fs + Fd) / cameraMass;
	glm::vec3 v = a * dt;
	setPos(getPos() + a * dt);
	if (getPos().y < cameraTargetLocation.y)
		cameraPos.y = cameraTargetLocation.y;


	if (glm::length(cameraTargetLocation - getPos()) > 4)
		setPos(cameraTargetLocation + glm::normalize(getPos() - cameraTargetLocation) * 4.f);

	cameraVelocity = currentCamLocation - getPos();

	setPos(cameraTargetLocation);
}
