#include "../GraphicsSystem.h"
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "core/ecs.h"
#include "../components.h"

#include "glm/gtc/quaternion.hpp"
#include <glm/gtx/quaternion.hpp>
#include "glm/gtx/projection.hpp"

#include "../PhysicsSystem.h"

using namespace physics;

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
	
	//cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * hspeed;
	//cameraPos += cameraSpeed * cameraFront * fspeed;

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
	//if the tracking entity has changed then set the camera to a good position behind it
	if (fixCamera) {
		glm::vec2 carFront2D;
		{
			glm::vec3 temp = _carTransform.getRotation() * glm::vec3(0, 0, 1);
			carFront2D = glm::normalize(glm::vec2(temp[0], temp[2]));
		}
		cameraTargetLocation = _carTransform.getTranslation() + glm::vec3(-carFront2D[0] * 20, _carTransform.getTranslation().y + GraphicsSystem::height_offset, -carFront2D[1] * 20);
		std::fill(rollingAverageCamera.begin(), rollingAverageCamera.end(), cameraTargetLocation);
		std::fill(rollingAverageDownvector.begin(), rollingAverageDownvector.end(), glm::vec3(0, -1, 0));//used to fix camera jitter on reset
		setPos(cameraTargetLocation);
		fixCamera = false;
	}

	/*
	* calculate the camera position
	*/
	/*
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
	*/
	//update the camera using a spring 
	//PxRaycastBuffer hitBuffer;
	//glm::vec3 cameraPlaneNormal = glm::vec3(0,1,0);
	//if (physics::physicsSystem.m_Scene->raycast(GLMtoPx(getPos()), PxVec3(0, -1, 0), 100, hitBuffer)) {
		//cameraPlaneNormal = PxtoGLM(hitBuffer.block.normal);
	//}
	//else {
	//	cameraPlaneNormal = _carTransform.getRotation() * glm::vec3(1, 0, 0);
	//}
	//calculate where the camera should aim to be positioned

	auto projectOntoPlane = [](glm::vec3 vec, glm::vec3 norm)
	{
		return vec - glm::proj(vec, norm);
	};

	//set up variables used in the calculations
	glm::vec3 carPos3D = _carTransform.getTranslation();
	glm::vec3 downVector = glm::vec3(0, 0, 0); 
	for (glm::vec3& e : rollingAverageDownvector) {
		downVector += e / (float)rollingAverageAmount;
	}
	downVector = glm::normalize(downVector);
	//perform a raycast down and figure out the normal of the road beneth the car
	PxQueryFilterData filterData(PxQueryFlag::eSTATIC);
	PxRaycastBuffer hitBuffer;
	if (physics::physicsSystem.m_Scene->raycast(GLMtoPx(carPos3D + (-downVector * 3.f)), GLMtoPx(downVector), 300, hitBuffer, PxHitFlag::eDEFAULT, filterData)) {
		rollingAverageDownvector[rollingIndex] = -PxtoGLM(hitBuffer.block.normal);
	}
	rollingIndex = rollingIndex == rollingAverageAmount - 1 ? 0 : rollingIndex + 1;
	//the positions of objects in the 2D plane defined by the down vector
	//the plane goes through 0,0 and is defined by the down vector
	
	//glm::vec3 carPos2D = projectOntoPlane(carPos3D, downVector);
	glm::vec3 camPos2D = carPos3D + projectOntoPlane(cameraTargetLocation - carPos3D, downVector);
	glm::vec3 carFront2D;
	{
		glm::vec3 temp = glm::toMat4(_carTransform.getRotation()) * glm::vec4(0, 0, 1, 1);
		carFront2D = glm::normalize(projectOntoPlane(temp, downVector)); //temp will never be 0 length since it is just being rotated
	}

	/*
	* determine the location of the camera target by performing operations in the xz plane
	*/
	//move the camera towards the centre axis of the car
	glm::vec3 CameraToCenterLine = (carPos3D - camPos2D) - glm::proj(carPos3D - camPos2D, carFront2D);
	if (glm::length(CameraToCenterLine) > GraphicsSystem::centering_slack_margin) {
		float travelDistance = GraphicsSystem::centering_speed * dt;
		travelDistance = glm::length(CameraToCenterLine) - GraphicsSystem::centering_slack_margin > travelDistance ? travelDistance : glm::length(CameraToCenterLine) - GraphicsSystem::centering_slack_margin;
		camPos2D += glm::normalize(CameraToCenterLine) * travelDistance;
	}
	if (acos(glm::dot(carFront2D, glm::normalize(camPos2D - carPos3D))) < glm::radians(GraphicsSystem::pushback_angle)) {
		//move the camera towards the back of the car
		camPos2D += -carFront2D * GraphicsSystem::pushback_strength * dt;
	}

	//if the target is in the innder radius then project it to the minimum radius
	if (glm::length(camPos2D - carPos3D) < GraphicsSystem::minimum_radius && glm::length(camPos2D - carPos3D) != 0) {
		camPos2D = (glm::normalize(camPos2D - carPos3D) * GraphicsSystem::minimum_radius) + carPos3D;
	}
	//if the target is outside the outer radius then project it back in
	if (glm::length(camPos2D - carPos3D) > GraphicsSystem::maximum_radius) {
		camPos2D = (glm::normalize(camPos2D - carPos3D) * GraphicsSystem::maximum_radius) + carPos3D;
	}

	//figure out the height of the camera by raycasting down and finding distance to the ground
	//first calculate the camera height respective to the car
	//float cameraHeight = glm::length(glm::proj(carPos3D, downVector) + (-downVector * GraphicsSystem::height_offset));
	float cameraHeight = GraphicsSystem::height_offset;
	//fix the sign since length always returns positive
	glm::vec3 cameraRaycastOrigin = camPos2D + (-downVector * cameraHeight) + (-downVector * 20.f);
	//if there is a collision with the ground figure out the camera height respective to the ground
	if (physics::physicsSystem.m_Scene->raycast(GLMtoPx(cameraRaycastOrigin), GLMtoPx(downVector), 260, hitBuffer, PxHitFlag::eDEFAULT, filterData)) {
		PxRaycastHit hit = hitBuffer.block;
		if (glm::dot(PxtoGLM(hit.normal), -downVector) > 0.3) {
			float collisionHeight = glm::distance(projectOntoPlane(PxtoGLM(hit.position) - carPos3D, downVector) + carPos3D, PxtoGLM(hit.position) + (-downVector * GraphicsSystem::height_offset));
			cameraHeight = std::max(cameraHeight, collisionHeight);
		}
	}

	//while there is not line of sight between car and camera try moving the camera target around the circle
	glm::vec3 camPosTEMP = camPos2D + cameraHeight * -downVector;
	glm::vec3 carToCam3D = camPosTEMP - carPos3D;
	glm::vec3 carToCamDir3D = glm::normalize(carToCam3D);
	float carToCamLength = glm::length(carToCam3D);

	if (physics::physicsSystem.m_Scene->raycast(GLMtoPx(carPos3D), GLMtoPx(carToCamDir3D), carToCamLength, hitBuffer, PxHitFlag::eDEFAULT, filterData)) {
		PxRaycastHit hit = hitBuffer.block;
		glm::vec3 collisionPoint = projectOntoPlane(PxtoGLM(hit.position) - carPos3D, downVector) + carPos3D;
		camPos2D = collisionPoint;
	}


	//move the camera towards the target
	cameraTargetLocation = camPos2D + -downVector * cameraHeight;
	rollingAverageCamera[cameraRollingIndex] = cameraTargetLocation;
	cameraRollingIndex = cameraRollingIndex == cameraRollingAverageAmount - 1 ? 0 : cameraRollingIndex + 1;
	//glm::vec3 movementDirection = glm::distance(cameraTargetLocation, getPos()) > 0.1 ? cameraTargetLocation - getPos() : glm::vec3(0);
	//float moveAmount = 10.f * dt;
	//moveAmount = glm::distance(cameraTargetLocation, getPos()) < moveAmount ? glm::distance(cameraTargetLocation, getPos()) : moveAmount;
	//setPos(getPos() + movementDirection * moveAmount);
	//if(glm::distance(cameraTargetLocation, getPos()) < 8 || glm::distance(cameraTargetLocation, getPos()) > 20)
	glm::vec3 finalPos = glm::vec3(0);
	for (glm::vec3& e : rollingAverageCamera) {
		finalPos += e / (float)cameraRollingAverageAmount;
	}
	setPos(finalPos);

}
