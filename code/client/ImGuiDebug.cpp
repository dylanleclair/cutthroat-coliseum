#include "ImGuiDebug.h"
#include <iostream>

void reloadVehicleJSON() {
	ImGui::Begin("Serialize");

	if (ImGui::Button("Reload Base.Json")) {
		readBaseParamsFromJsonFile("../../../../assets/vehicledata", "Base.json", gVehicle.mBaseParams);
		
		//gVehicle.mBaseParams.rigidBodyParams.mass = 300.0f;
		std::cout << gVehicle.mBaseParams.rigidBodyParams.mass << std::endl;
	}

	if (ImGui::Button("Reload EngineDrive.Json")) {
		readEngineDrivetrainParamsFromJsonFile(gVehicleDataPath, "EngineDrive.json",
			gVehicle.mEngineDriveParams);
	}
	
	ImGui::End();
}