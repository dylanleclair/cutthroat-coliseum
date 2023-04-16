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
	//PxShape* sphere = physicsSystem.m_Physics->createShape(PxSphereGeometry(1.f), *(physics::physicsSystem.m_Material), true);
	//PxTransform localTm(PxVec3(-90, 20, 0));
	//body = physicsSystem.m_Physics->createRigidDynamic(localTm);
	//body->attachShape(*sphere);
	//body->setMass(0);
	//body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	//physicsSystem.m_Scene->addActor(*body);
	//sphere->release();
	//cameraPos.attachActor(body);
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

	//set up variables used in the calculations
	glm::vec2 carPos2D = glm::vec2(_carTransform.getTranslation()[0], _carTransform.getTranslation()[2]);
	glm::vec2 camPosT2D = glm::vec2(cameraTargetLocation[0], cameraTargetLocation[2]);
	glm::vec2 carVel2D = glm::vec2(carVelocity[0], carVelocity[2]);
	glm::vec2 carFront2D;
	{
		glm::vec4 temp = glm::toMat4(_carTransform.getRotation()) * glm::vec4(0, 0, 1, 1);
		carFront2D = glm::normalize(glm::vec2(temp[0], temp[2]));
	}
	glm::vec3 carPos3D = _carTransform.getTranslation();
	/*
	* determine the location of the camera target by performing operations in the xz plane
	*/
	//move the camera towards the centre axis of the car
	glm::vec2 CameraToCenterLine = -((camPosT2D - carPos2D) - glm::proj(camPosT2D - carPos2D, carFront2D));
	if (glm::length(CameraToCenterLine) > GraphicsSystem::centering_slack_margin) {
		float travelDistance = GraphicsSystem::centering_speed * dt;
		travelDistance = glm::length(CameraToCenterLine) - GraphicsSystem::centering_slack_margin > travelDistance ? travelDistance : glm::length(CameraToCenterLine) - GraphicsSystem::centering_slack_margin;
		camPosT2D += glm::normalize(CameraToCenterLine) * travelDistance;
	}
	if (acos(glm::dot(carFront2D, glm::normalize(camPosT2D - carPos2D))) > glm::radians(GraphicsSystem::pushback_angle = 95)) {
		//move the camera towards the back of the car
		camPosT2D += -carFront2D * GraphicsSystem::pushback_strength * dt;
	}

	//if the target is in the innder radius then project it to the minimum radius
	
	if (glm::length(camPosT2D - carPos2D) < GraphicsSystem::minimum_radius && glm::length(camPosT2D - carPos2D) != 0) {
		camPosT2D = (glm::normalize(camPosT2D - carPos2D) * GraphicsSystem::minimum_radius) + carPos2D;
	}
	//if the target is outside the outer radius then project it back in
	if (glm::length(camPosT2D - carPos2D) > GraphicsSystem::maximum_radius) {
		camPosT2D = (glm::normalize(camPosT2D - carPos2D) * GraphicsSystem::maximum_radius) + carPos2D;
	}

	//figure out the height of the camera by raycasting down and finding distance to the ground
	PxQueryFilterData filterData(PxQueryFlag::eSTATIC);
	PxRaycastBuffer hitBuffer;
	float cameraHeight = carPos3D.y + GraphicsSystem::height_offset;
	if (physics::physicsSystem.m_Scene->raycast(GLMtoPx(glm::vec3(camPosT2D[0], carPos3D.y + 40, camPosT2D[1])), PxVec3(0,-1,0), 260, hitBuffer)) {
		PxRaycastHit hit = hitBuffer.block;
		if(glm::dot(PxtoGLM(hit.normal), glm::vec3(0, 1, 0)) > 0.3)
			cameraHeight = std::max(cameraHeight, hit.position.y + GraphicsSystem::height_offset);
	}
	

	//while there is not line of sight between car and camera try moving the camera target around the circle
	glm::vec3 carToCam3D = glm::vec3(camPosT2D[0], cameraHeight, camPosT2D[1]) - carPos3D;
	glm::vec3 carToCamDir3D = glm::normalize(carToCam3D);
	float carToCamLength = glm::length(carToCam3D);

	//glm::vec2 originalDir2D = glm::vec2(carToCamDir3D[0], carToCamDir3D[2]);
	//float angleOff = 0; //radians
	//bool clockwise = 0; //0 = counterclockwise, 1 = clockwise
	if (physics::physicsSystem.m_Scene->raycast(GLMtoPx(carPos3D), GLMtoPx(carToCamDir3D), carToCamLength, hitBuffer, PxHitFlag::eDEFAULT, filterData)) {
		PxRaycastHit hit = hitBuffer.block;
		glm::vec2 collisionPoint = glm::vec2(hit.position.x, hit.position.z);
		camPosT2D = collisionPoint;
		/*
		//test if just moving the camera closer would solve it
		glm::vec2 collisionPoint = glm::vec2(hit.position.x, hit.position.z);
		if (glm::length(collisionPoint - carPos2D) > minDistance) {
			camPosT2D = collisionPoint;
			break;
		}
		
		//update the values for the next raycast
		//rotate the direction of the ray
		char dirMod = clockwise * 2 - 1;
		//rotate the original vector from the car to camera by a angle to try and find a solution
		glm::vec2 newDir = glm::mat2(cos(dirMod * angleOff), sin(dirMod * angleOff), -sin(dirMod * angleOff), cos(dirMod * angleOff)) * originalDir2D;
		carToCamDir3D = glm::normalize(glm::vec3(,cameraHeight,)  - carPos3D);
		if (clockwise)
			angleOff += 0.3;
		clockwise = !clockwise;*/
	}

	//move the camera towards the target
	cameraTargetLocation = glm::vec3(camPosT2D[0], cameraHeight, camPosT2D[1]);
	glm::vec3 movementDirection = glm::distance(cameraTargetLocation, getPos()) > 0.1 ? cameraTargetLocation - getPos() : glm::vec3(0);
	float moveAmount = 10.f * dt;
	moveAmount = glm::distance(cameraTargetLocation, getPos()) < moveAmount ? glm::distance(cameraTargetLocation, getPos()) : moveAmount;
	//setPos(getPos() + movementDirection * moveAmount);
	setPos(cameraTargetLocation);

}
