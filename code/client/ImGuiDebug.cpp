#include "ImGuiDebug.h"
#include <iostream>
#include "Car.h"

void reloadVehicleJSON(Car& car) {
	ImGui::Begin("Serialize");

	if (ImGui::Button("Reload Base.Json")) {
		readBaseParamsFromJsonFile("../../../../assets/vehicledata", "Base.json", car.m_Vehicle.mBaseParams);
		
		//gVehicle.mBaseParams.rigidBodyParams.mass = 300.0f;
		std::cout << car.m_Vehicle.mBaseParams.rigidBodyParams.mass << std::endl;
	}

	if (ImGui::Button("Reload EngineDrive.Json")) {
		readEngineDrivetrainParamsFromJsonFile(gVehicleDataPath, "EngineDrive.json",
			car.m_Vehicle.mEngineDriveParams);
	}
	
	ImGui::End();
}