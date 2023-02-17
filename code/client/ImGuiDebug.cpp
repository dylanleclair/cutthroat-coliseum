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
	brake_max = gVehicle.mBaseParams.brakeResponseParams->maxResponse;
	wheel_response_multip = gVehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers;
}

void vehicleTuning() {
	ImGui::Begin("Tuning");

	if (ImGui::TreeNode("Rigid Body:")) {

		if (ImGui::InputFloat("Mass: (in kg)", &rigid_mass)) {
			gVehicle.mBaseParams.rigidBodyParams.mass = rigid_mass;
		}

		if (ImGui::TreeNode("MOI: (affects turning and rolling)")) {
			ImGui::Text("Higher values make it harder to induce rotation speed from tire, and suspension forces");
			ImGui::Text("Physical values: x= (L^2 + H^2)*M/12, y=(W^2+L^2)*M/12, z=(H^2+W^2)*M/12");
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

	if (ImGui::TreeNode("Brake Command Reponse Params:")) {
		ImGui::Text("Higher torque locks wheels quicker - Value strongly related to wheel MOI");
		if (ImGui::InputFloat("Max Response:", &brake_max)) {			
			gVehicle.mBaseParams.brakeResponseParams->maxResponse = brake_max;
		}

		if (ImGui::TreeNode("Wheel Response Multipliers")) {
			ImGui::Text("Left and right might not be in the right order, front and left and rear are correct");
			ImGui::Text("These are multipliers, 0.0 means the brake is disabled, 1.0 is full brake power");

			if (ImGui::SliderFloat("Front Left Wheel:", &wheel_response_multip[0], 0.f, 1.f)) {
				gVehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers[0] = wheel_response_multip[0];
			}
			if (ImGui::SliderFloat("Front Right Wheel:", &wheel_response_multip[1], 0.f, 1.f)) {
				gVehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers[1] = wheel_response_multip[1];
			}
			if (ImGui::SliderFloat("Rear Left Wheel:", &wheel_response_multip[2], 0.f, 1.f)) {
				gVehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers[2] = wheel_response_multip[2];
			}
			if (ImGui::SliderFloat("Read Right Wheel:", &wheel_response_multip[3], 0.f, 1.f)) {
				gVehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers[3] = wheel_response_multip[3];
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