#include "TireTracks.h"
#include "AICar.h"

std::vector<std::vector<VFXTextureStrip>> tireTrackCol;
std::vector<std::vector<bool>> previousStates;

// Sets up tire tracks ecs entities
// Needs a unique one for each driver
void setupTireTrackVisuals(ecs::Scene& mainScene, int number_of_vehicles) {
	std::vector<VFXTextureStrip> tireTrackVisuals;

	for (int i = 0; i < number_of_vehicles; i++) {
		//front tire
		ecs::Entity frontTireTrack = mainScene.CreateEntity();
		VFXTextureStrip frontTireTrack_r = VFXTextureStrip("textures/MotercycleTireTread.png", 0.07, 2);
		frontTireTrack_r.maxLength = 35;
		TransformComponent frontTireTrack_t = TransformComponent();
		mainScene.AddComponent(frontTireTrack.guid, frontTireTrack_r);
		mainScene.AddComponent(frontTireTrack.guid, frontTireTrack_t);

		// Putting it into the collection for out of scope use
		VFXTextureStrip& frontTireT = mainScene.GetComponent<VFXTextureStrip>(frontTireTrack.guid);
		tireTrackVisuals.push_back(frontTireT);

		//right tire
		ecs::Entity rightTireTrack = mainScene.CreateEntity();
		VFXTextureStrip rightTireTrack_r = VFXTextureStrip("textures/MotercycleTireTread.png", 0.07, 1);
		rightTireTrack_r.maxLength = 25;
		TransformComponent rightTireTrack_t = TransformComponent();
		mainScene.AddComponent(rightTireTrack.guid, rightTireTrack_r);
		mainScene.AddComponent(rightTireTrack.guid, rightTireTrack_t);

		// Putting it into the collection for out of scope use
		VFXTextureStrip& rightTireT = mainScene.GetComponent<VFXTextureStrip>(rightTireTrack.guid);
		tireTrackVisuals.push_back(rightTireT);

		//left tire
		ecs::Entity leftTireTrack = mainScene.CreateEntity();
		VFXTextureStrip leftTireTrack_r = VFXTextureStrip("textures/MotercycleTireTread.png", 0.07, 1);
		leftTireTrack_r.maxLength = 25;
		TransformComponent leftTireTrack_t = TransformComponent();
		mainScene.AddComponent(leftTireTrack.guid, leftTireTrack_r);
		mainScene.AddComponent(leftTireTrack.guid, leftTireTrack_t);

		// Putting it into the collection for out of scope use
		VFXTextureStrip& leftTireT = mainScene.GetComponent<VFXTextureStrip>(leftTireTrack.guid);
		tireTrackVisuals.push_back(leftTireT);

		tireTrackCol.push_back(tireTrackVisuals);
		tireTrackVisuals.clear();
	}

	setupPreviousStates(number_of_vehicles);
}

// Sets up previous state checks for the render loop
// needs a unique one for each driver
void setupPreviousStates(int number_of_vehicles) {
	std::vector<bool> previousState;
	previousState.push_back(false);
	previousState.push_back(false);
	previousState.push_back(false);

	for (int i = 0; i < number_of_vehicles; i++) {
		previousStates.push_back(previousState);
	}	
}

void TireTracks(Car& car, std::vector<Guid> &AIGuids, ecs::Scene mainScene) {

	// For the player car 
	std::vector<VFXTextureStrip> tracks = tireTrackCol.at(0);

	VFXTextureStrip& frontTireTracks = tracks.at(0);
	VFXTextureStrip& rightTireTracks = tracks.at(1);
	VFXTextureStrip& leftTireTracks = tracks.at(2);

	//VFXTextureStrip& frontTireTracks = mainScene.GetComponent<VFXTextureStrip>(frontTireT.guid);
	//VFXTextureStrip& rightTireTracks = mainScene.GetComponent<VFXTextureStrip>(rightTireTrack.guid);
	//VFXTextureStrip& leftTireTracks = mainScene.GetComponent<VFXTextureStrip>(leftTireTrack.guid);

	//front tire
	if (car.m_Vehicle.mBaseState.roadGeomStates[0].hitState && car.m_Vehicle.mBaseState.roadGeomStates[1].hitState) {
		previousStates.at(0).at(0) = true;
		glm::vec3 frontTirePosition = PxtoGLM(car.getVehicleRigidBody()->getGlobalPose().p) + glm::vec3(PxtoGLM(car.getVehicleRigidBody()->getGlobalPose().q) * glm::vec4(0, -.3, 4, 1));
		if (glm::length(frontTirePosition - frontTireTracks.g_previousPosition()) > 1) {
			frontTireTracks.extrude(frontTirePosition, glm::vec3(0, 1, 0));
		}
		else {
			//frontTireTracks.moveEndPoint(frontTirePosition, glm::vec3(0, 1, 0));
		}
	}
	else {
		if (previousStates.at(0).at(0) == true)
			frontTireTracks.cut();
		previousStates.at(0).at(0) = false;
	}

	//right tire
	if (car.m_Vehicle.mBaseState.roadGeomStates[2].hitState) {
		previousStates.at(0).at(1) = true;
		glm::vec3 rightTirePosition = PxtoGLM(car.getVehicleRigidBody()->getGlobalPose().p) + glm::vec3(PxtoGLM(car.getVehicleRigidBody()->getGlobalPose().q) * glm::vec4(1, -.3, 1, 1));
		if (glm::length(rightTirePosition - rightTireTracks.g_previousPosition()) > 1) {
			rightTireTracks.extrude(rightTirePosition, glm::vec3(0, 1, 0));
		}
		else {
			//rightTireTracks.moveEndPoint(rightTirePosition, glm::vec3(0, 1, 0));
		}
	}
	else {
		if (previousStates.at(0).at(1) == true)
			rightTireTracks.cut();
		previousStates.at(0).at(1) = false;
	}

	//left tire
	if (car.m_Vehicle.mBaseState.roadGeomStates[3].hitState) {
		previousStates.at(0).at(2) = true;
		glm::vec3 leftTirePosition = PxtoGLM(car.getVehicleRigidBody()->getGlobalPose().p) + glm::vec3(PxtoGLM(car.getVehicleRigidBody()->getGlobalPose().q) * glm::vec4(-1, -.3, 1, 1));
		if (glm::length(leftTirePosition - leftTireTracks.g_previousPosition()) > 1) {
			leftTireTracks.extrude(leftTirePosition, glm::vec3(0, 1, 0));
		}
		else {
			//leftTireTracks.moveEndPoint(leftTirePosition, glm::vec3(0, 1, 0));
		}
	}
	else {
		if (previousStates.at(0).at(2) == true)
			leftTireTracks.cut();
		previousStates.at(0).at(2) = false;
	}

	//For all the AI cars

	AICar& AICarTest = mainScene.GetComponent<AICar>(AIGuids.at(0));
}