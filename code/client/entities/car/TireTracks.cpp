#include "TireTracks.h"
#include "AICar.h"

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
	//front tire
	ecs::Entity frontTireTrack = mainScene.CreateEntity();
	VFXTextureStrip frontTireTrack_r = VFXTextureStrip("textures/MotercycleTireTread.png", 0.07, 2);
	frontTireTrack_r.maxLength = 115;
	TransformComponent frontTireTrack_t = TransformComponent();
	mainScene.AddComponent(frontTireTrack.guid, frontTireTrack_r);
	mainScene.AddComponent(frontTireTrack.guid, frontTireTrack_t);

	//right tire
	ecs::Entity rightTireTrack = mainScene.CreateEntity();
	VFXTextureStrip rightTireTrack_r = VFXTextureStrip("textures/MotercycleTireTread.png", 0.07, 1);
	rightTireTrack_r.maxLength = 100;
	TransformComponent rightTireTrack_t = TransformComponent();
	mainScene.AddComponent(rightTireTrack.guid, rightTireTrack_r);
	mainScene.AddComponent(rightTireTrack.guid, rightTireTrack_t);
		
	//left tire
	ecs::Entity leftTireTrack = mainScene.CreateEntity();
	VFXTextureStrip leftTireTrack_r = VFXTextureStrip("textures/MotercycleTireTread.png", 0.07, 1);
	leftTireTrack_r.maxLength = 100;
	TransformComponent leftTireTrack_t = TransformComponent();
	mainScene.AddComponent(leftTireTrack.guid, leftTireTrack_r);
	mainScene.AddComponent(leftTireTrack.guid, leftTireTrack_t);

	//store the Guids
	bool isAI = false;
	if (mainScene.HasComponent<AICar>(_ID))
		isAI = true;
	VFXGuids.push_back({ _ID, frontTireTrack.guid, rightTireTrack.guid, leftTireTrack.guid, isAI });
	previousStates.push_back({ false, false, false });
}

void updateCarVFX(ecs::Scene mainScene) {
	float y_offset = -0.32f; // tire tracks will be rendered not on the ground plane otherwise
	// if same offset is used the AI car tracks will be inside the ground
	for (int i = 0; i < VFXGuids.size(); i++) {
		attachedVFX vfx = VFXGuids[i];
		Car* car;
		if (vfx.isAI) {
			car = &mainScene.GetComponent<AICar>(vfx.ownerGuid);
		}
		else {
			car = &mainScene.GetComponent<Car>(vfx.ownerGuid);
		}

		VFXTextureStrip& frontTireTracks = mainScene.GetComponent<VFXTextureStrip>(vfx.trackGuids[0]);
		VFXTextureStrip& rightTireTracks = mainScene.GetComponent<VFXTextureStrip>(vfx.trackGuids[1]);
		VFXTextureStrip& leftTireTracks = mainScene.GetComponent<VFXTextureStrip>(vfx.trackGuids[2]);

		if (car->m_Vehicle.mBaseState.roadGeomStates[0].hitState && car->m_Vehicle.mBaseState.roadGeomStates[1].hitState) {
			previousStates.at(i).at(0) = true;
			glm::vec3 frontTirePosition = PxtoGLM(car->getVehicleRigidBody()->getGlobalPose().p) + glm::vec3(PxtoGLM(car->getVehicleRigidBody()->getGlobalPose().q) * glm::vec4(0, y_offset, 4, 1));
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
			glm::vec3 rightTirePosition = PxtoGLM(car->getVehicleRigidBody()->getGlobalPose().p) + glm::vec3(PxtoGLM(car->getVehicleRigidBody()->getGlobalPose().q) * glm::vec4(1, y_offset, 1, 1));
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
			glm::vec3 leftTirePosition = PxtoGLM(car->getVehicleRigidBody()->getGlobalPose().p) + glm::vec3(PxtoGLM(car->getVehicleRigidBody()->getGlobalPose().q) * glm::vec4(-1, y_offset, 1, 1));
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

	}
	
}