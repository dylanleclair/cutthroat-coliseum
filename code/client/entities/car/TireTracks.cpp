#include "TireTracks.h"
#include "AICar.h"
#include <iostream>

/*
* put them in a struct for easier code readability and easier expandability for future effects
*/
struct attachedVFX {
	Guid ownerGuid;
	Guid trackGuids[3];
	bool isAI;
};

std::vector<std::array<bool, 3>> previousStates;
std::vector<attachedVFX> VFXGuids;

// Sets up tire tracks ecs entities
// Needs a unique one for each driver
void setupCarVFX(ecs::Scene& mainScene, Guid _ID) {
	bool isAI = false;
	if (mainScene.HasComponent<AICar>(_ID))
		isAI = true;
	Car* car;
	if (isAI)
		car = &mainScene.GetComponent<AICar>(_ID);
	else
		car = &mainScene.GetComponent<Car>(_ID);


	//front tire
	ecs::Entity frontTireTrack = mainScene.CreateEntity();
	VFXTextureStrip frontTireTrack_r = VFXTextureStrip("textures/MotercycleTireTread.png", 0.07, 2);
	frontTireTrack_r.maxLength = 115;
	TransformComponent frontTireTrack_t = TransformComponent(car->getVehicleRigidBody());
	frontTireTrack_t.setPosition(glm::vec3(0, 0, 3.2));
	mainScene.AddComponent(frontTireTrack.guid, frontTireTrack_r);
	mainScene.AddComponent(frontTireTrack.guid, frontTireTrack_t);

	//right tire
	ecs::Entity rightTireTrack = mainScene.CreateEntity();
	VFXTextureStrip rightTireTrack_r = VFXTextureStrip("textures/MotercycleTireTread.png", 0.07, 1);
	rightTireTrack_r.maxLength = 100;
	TransformComponent rightTireTrack_t = TransformComponent(car->getVehicleRigidBody());
	rightTireTrack_t.setPosition(glm::vec3(-1, 0, -0.65));
	mainScene.AddComponent(rightTireTrack.guid, rightTireTrack_r);
	mainScene.AddComponent(rightTireTrack.guid, rightTireTrack_t);
		
	//left tire
	ecs::Entity leftTireTrack = mainScene.CreateEntity();
	VFXTextureStrip leftTireTrack_r = VFXTextureStrip("textures/MotercycleTireTread.png", 0.07, 1);
	leftTireTrack_r.maxLength = 100;
	TransformComponent leftTireTrack_t = TransformComponent(car->getVehicleRigidBody());
	leftTireTrack_t.setPosition(glm::vec3(1, 0, -0.65));
	mainScene.AddComponent(leftTireTrack.guid, leftTireTrack_r);
	mainScene.AddComponent(leftTireTrack.guid, leftTireTrack_t);

	//set up the particle system
	VFXParticleSystem frontParticles = VFXParticleSystem("textures/dustParticle.png", 50);
	frontParticles.initialPositionMin = glm::vec3(0, 0, 0);
	frontParticles.initialPositionMax = glm::vec3(0, 0, 0);
	frontParticles.initialVelocityMin = glm::vec3(10, 0.5, 0);
	frontParticles.initialVelocityMax = glm::vec3(-10, 0.7, 0);
	frontParticles.alphaChangeMin = -0.6;
	frontParticles.alphaChangeMax = -1.2;
	frontParticles.scaleSpeedMin = -0.7;
	frontParticles.scaleSpeedMax = -1;
	frontParticles.particleLife = 1;
	mainScene.AddComponent(frontTireTrack.guid, frontParticles);

	VFXParticleSystem rightParticles = VFXParticleSystem("textures/dustParticle.png", 50);
	rightParticles.initialPositionMin = glm::vec3(0, 0, 0);
	rightParticles.initialPositionMax = glm::vec3(0, 0, 0);
	rightParticles.initialVelocityMin = glm::vec3(10, 0.5, 0);
	rightParticles.initialVelocityMax = glm::vec3(-10, 0.7, 0);
	rightParticles.alphaChangeMin = -0.6;
	rightParticles.alphaChangeMax = -1.2;
	rightParticles.scaleSpeedMin = -0.7;
	rightParticles.scaleSpeedMax = -1;
	rightParticles.particleLife = 1;
	mainScene.AddComponent(rightTireTrack.guid, rightParticles);

	VFXParticleSystem leftParticles = VFXParticleSystem("textures/dustParticle.png", 50);
	leftParticles.initialPositionMin = glm::vec3(0, 0, 0);
	leftParticles.initialPositionMax = glm::vec3(0, 0, 0);
	leftParticles.initialVelocityMin = glm::vec3(10, 0.5, 0);
	leftParticles.initialVelocityMax = glm::vec3(-10, 0.7, 0);
	leftParticles.alphaChangeMin = -0.6;
	leftParticles.alphaChangeMax = -1.2;
	leftParticles.scaleSpeedMin = -0.7;
	leftParticles.scaleSpeedMax = -1;
	leftParticles.particleLife = 1;
	mainScene.AddComponent(leftTireTrack.guid, leftParticles);

	VFXGuids.push_back({ _ID, frontTireTrack.guid, rightTireTrack.guid, leftTireTrack.guid, isAI });
	previousStates.push_back({ false, false, false });
}

void updateCarVFX(ecs::Scene mainScene, float dt) {
	float y_offset = -0.32; // offset needs to be different for Player and AI cars
	// if same offset is used the AI car tracks will be inside the ground
	for (int i = 0; i < VFXGuids.size(); i++) {
		attachedVFX vfx = VFXGuids[i];
		Car* car;
		if (vfx.isAI) {
			car = &mainScene.GetComponent<AICar>(vfx.ownerGuid);
			y_offset = 0.0f;
		}
		else {
			car = &mainScene.GetComponent<Car>(vfx.ownerGuid);
			y_offset = -0.32f;
		}

		VFXTextureStrip& frontTireTracks = mainScene.GetComponent<VFXTextureStrip>(vfx.trackGuids[0]);
		VFXTextureStrip& rightTireTracks = mainScene.GetComponent<VFXTextureStrip>(vfx.trackGuids[1]);
		VFXTextureStrip& leftTireTracks = mainScene.GetComponent<VFXTextureStrip>(vfx.trackGuids[2]);

		TransformComponent& frontTireTransform = mainScene.GetComponent<TransformComponent>(vfx.trackGuids[0]);
		TransformComponent& rightTireTransform = mainScene.GetComponent<TransformComponent>(vfx.trackGuids[1]);
		TransformComponent& leftTireTransform = mainScene.GetComponent<TransformComponent>(vfx.trackGuids[2]);

		if (car->m_Vehicle.mBaseState.roadGeomStates[0].hitState && car->m_Vehicle.mBaseState.roadGeomStates[1].hitState) {
			previousStates.at(i).at(0) = true;
			glm::vec3 frontTirePosition = frontTireTransform.getTransformationMatrix() * glm::vec4(0, y_offset, 0, 1);
			if (glm::length(frontTirePosition - frontTireTracks.g_previousPosition()) > 1) {
				frontTireTracks.extrude(frontTirePosition, glm::vec3(0, 1, 0));
			}
			else {
				//frontTireTracks.moveEndPoint(frontTirePosition, glm::vec3(0, 1, 0));
			}
		}
		else {
			if (previousStates.at(i).at(0) == true)
				frontTireTracks.cut();
			previousStates.at(i).at(0) = false;
		}

		//right tire
		if (car->m_Vehicle.mBaseState.roadGeomStates[2].hitState) {
			previousStates.at(i).at(1) = true;
			glm::vec3 rightTirePosition = rightTireTransform.getTransformationMatrix() * glm::vec4(0, y_offset, 0, 1);
			if (glm::length(rightTirePosition - rightTireTracks.g_previousPosition()) > 1) {
				rightTireTracks.extrude(rightTirePosition, glm::vec3(0, 1, 0));
			}
			else {
				//rightTireTracks.moveEndPoint(rightTirePosition, glm::vec3(0, 1, 0));
			}
		}
		else {
			if (previousStates.at(i).at(1) == true)
				rightTireTracks.cut();
			previousStates.at(i).at(1) = false;
		}

		//left tire
		if (car->m_Vehicle.mBaseState.roadGeomStates[3].hitState) {
			previousStates.at(i).at(2) = true;
			glm::vec3 leftTirePosition = leftTireTransform.getTransformationMatrix() * glm::vec4(0, y_offset, 0, 1);
			if (glm::length(leftTirePosition - leftTireTracks.g_previousPosition()) > 1) {
				leftTireTracks.extrude(leftTirePosition, glm::vec3(0, 1, 0));
			}
			else {
				//leftTireTracks.moveEndPoint(leftTirePosition, glm::vec3(0, 1, 0));
			}
		}
		else {
			if (previousStates.at(i).at(2) == true)
				leftTireTracks.cut();
			previousStates.at(i).at(2) = false;
		}



		//tire animation logic
		RenderModel& carModel = mainScene.GetComponent<RenderModel>(vfx.ownerGuid);
		glm::mat4 frontWheelRotation = glm::toMat4(PxtoGLM(car->m_Vehicle.mBaseState.wheelLocalPoses[0].localPose.q));
		glm::mat4 frontWheelTranslation = glm::translate(glm::mat4(1), glm::vec3(-0.026044, -0.18433, -0.92589));
		carModel.setMeshLocalTransformation(glm::inverse(frontWheelTranslation) * frontWheelRotation * frontWheelTranslation, "frontWheel");

		glm::mat4 rightWheelRotation = glm::toMat4(PxtoGLM(car->m_Vehicle.mBaseState.wheelLocalPoses[2].localPose.q));
		glm::mat4 rightWheelTranslation = glm::translate(glm::mat4(1), glm::vec3(0.31741, -0.18433, 0.28755));
		carModel.setMeshLocalTransformation(glm::inverse(rightWheelTranslation) * rightWheelRotation * rightWheelTranslation, "rightWheel");

		glm::mat4 leftWheelRotation = glm::toMat4(PxtoGLM(car->m_Vehicle.mBaseState.wheelLocalPoses[3].localPose.q));
		glm::mat4 leftWheelTranslation = glm::translate(glm::mat4(1), glm::vec3(-0.31741, -0.18433, 0.28755));
		carModel.setMeshLocalTransformation(glm::inverse(leftWheelTranslation) * leftWheelRotation * leftWheelTranslation, "leftWheel");

		//particle system
		
		VFXParticleSystem& frontTireParticle = mainScene.GetComponent<VFXParticleSystem>(vfx.trackGuids[0]);
		VFXParticleSystem& rightTireParticle = mainScene.GetComponent<VFXParticleSystem>(vfx.trackGuids[1]);
		VFXParticleSystem& leftTireParticle = mainScene.GetComponent<VFXParticleSystem>(vfx.trackGuids[2]);

		if (!previousStates[i][0])
			frontTireParticle.active = false;
		else
			frontTireParticle.active = true;

		if (!previousStates[i][1])
			rightTireParticle.active = false;
		else
			rightTireParticle.active = true;

		if (!previousStates[i][2])
			leftTireParticle.active = false;
		else
			leftTireParticle.active = true;

		const float speed = glm::length(PxtoGLM(car->getVehicleRigidBody()->getLinearVelocity()));
		const float frequency = (0.05 * speed * speed + 30) / (speed * speed + 20);
		frontTireParticle.particleFrequency = frequency;
		rightTireParticle.particleFrequency = frequency;
		leftTireParticle.particleFrequency = frequency;


		frontTireParticle.stepSystem(dt, frontTireTransform.getTransformationMatrix());
		rightTireParticle.stepSystem(dt, rightTireTransform.getTransformationMatrix());
		leftTireParticle.stepSystem(dt, leftTireTransform.getTransformationMatrix());
		
	}
	
}