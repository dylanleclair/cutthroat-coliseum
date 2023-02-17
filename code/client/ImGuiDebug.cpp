#include "ImGuiDebug.h"
#include <iostream>

// Initializes variables
void variableInit() {
	rigid_mass = gVehicle.mBaseParams.rigidBodyParams.mass;
}

void vehicleTuning() {
	ImGui::Begin("Tuning");

	if (ImGui::InputFloat("Mass", &rigid_mass)) {
		gVehicle.mBaseParams.rigidBodyParams.mass = rigid_mass;
	}

	ImGui::End();
}

void reloadVehicleJSON() {
	ImGui::Begin("Vehicle Variables");



	// Reads the JSON in the assets vehicle data
	// If first param set to gVehicleDataPath it goes to the out folder path instead
	if (ImGui::Button("Read Base.Json")) {
		readBaseParamsFromJsonFile("../../../../assets/vehicledata", "Base.json", gVehicle.mBaseParams);
	}

	// Reads the JSON in the assets vehicle data
	// If first param set to gVehicleDataPath it goes to the out folder path instead
	if (ImGui::Button("Read EngineDrive.Json")) {
		readEngineDrivetrainParamsFromJsonFile("../../../../assets/vehicledata", "EngineDrive.json", gVehicle.mEngineDriveParams);
	}
	
	ImGui::End();
}