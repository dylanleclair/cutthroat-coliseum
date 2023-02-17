#include "ImGuiDebug.h"
#include <iostream>

//if (ImGui::TreeNode("Bullets"))
//{
//	ImGui::BulletText("Bullet point 1");
//	ImGui::BulletText("Bullet point 2\nOn multiple lines");
//	if (ImGui::TreeNode("Tree node"))
//	{
//		ImGui::BulletText("Another bullet point");
//		ImGui::TreePop();
//	}
//	ImGui::Bullet(); ImGui::Text("Bullet point 3 (two calls)");
//	ImGui::Bullet(); ImGui::SmallButton("Button");
//	ImGui::TreePop();
//}

// Initializes variables
void variableInit() {
	rigid_mass = gVehicle.mBaseParams.rigidBodyParams.mass;
	rigid_MOI = gVehicle.mBaseParams.rigidBodyParams.moi;
}

void vehicleTuning() {
	ImGui::Begin("Tuning");

	if (ImGui::TreeNode("Rigid Body:")) {

		if (ImGui::InputFloat("Mass", &rigid_mass)) {
			gVehicle.mBaseParams.rigidBodyParams.mass = rigid_mass;
		}

		if (ImGui::TreeNode("MOI:")) {
			if (ImGui::InputFloat("MOI x: ", &rigid_MOI.x)) {
				gVehicle.mBaseParams.rigidBodyParams.moi.x = rigid_MOI.x;
			}
			if (ImGui::InputFloat("MOI y: ", &rigid_MOI.y)) {
				gVehicle.mBaseParams.rigidBodyParams.moi.y = rigid_MOI.y;
			}
			if (ImGui::InputFloat("MOI z: ", &rigid_MOI.z)) {
				gVehicle.mBaseParams.rigidBodyParams.moi.z = rigid_MOI.z;
			}
			ImGui::TreePop();
		}
		ImGui::TreePop();
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