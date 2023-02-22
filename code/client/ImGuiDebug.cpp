#include "ImGuiDebug.h"
#include <iostream>

// Initializes variables
void variableInit() {
	rigid_mass = gVehicle.mBaseParams.rigidBodyParams.mass;
	rigid_MOI = gVehicle.mBaseParams.rigidBodyParams.moi;

	brake_max = gVehicle.mBaseParams.brakeResponseParams->maxResponse;
	brake_response_multip = gVehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers;

	steer_max_response = gVehicle.mBaseParams.steerResponseParams.maxResponse;
	steer_multiplier = gVehicle.mBaseParams.steerResponseParams.wheelResponseMultipliers;

	number_of_wheels = 4;
	// Grabs values from each wheel and assigns it to variables for this file to edit
	for (int i = 0; i < number_of_wheels; i++) {
		wheel_radius[i] = gVehicle.mBaseParams.wheelParams[i].radius;
		wheel_half_width[i] = gVehicle.mBaseParams.wheelParams[i].halfWidth;
		wheel_mass[i] = gVehicle.mBaseParams.wheelParams[i].mass;
		wheel_moi[i] = gVehicle.mBaseParams.wheelParams[i].moi;
		wheel_dampening[i] = gVehicle.mBaseParams.wheelParams[i].dampingRate;
	}

	// Unused wheel ID's supposed be a pair [0,1] means front wheels, [2,3] means back wheels
	// The pair is a designation of which wheel pair is the ackerman angle
	//ack_wheel_ids = gVehicle.mBaseParams.ackermannParams->wheelIds;
	ack_wheel_base = gVehicle.mBaseParams.ackermannParams->wheelBase;
	ack_track_width = gVehicle.mBaseParams.ackermannParams->trackWidth;
	ack_strength = gVehicle.mBaseParams.ackermannParams->strength;
	
}

// Possible other tuning to add - these are not necessary for the game:
// Ackerman Angle (Rotates wheels in the zx direction when turning)
// Axle (Might be important if custom vehicle), (changes the size, length ect.. of axle)
// Suspension Params (How much suspension squishes and in which direction)
// Suspension State Calc (Leads to better simulation results, more computationally expensive)
// Suspension Compliance (Complex variables to help the car straighten after a turn)
// Suspension Force Params
// Anti Roll Bar
// Tire Force (Slip Params)
// 
// Engine Params (Engine Torques)
// Gearbox Params
// Autobox Params
// Clutch Command Response
// Differentials 


void vehicleTuning() {
	ImGui::Begin("Tuning");

	// Rigid Body params
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

	// Brake Commands
	if (ImGui::TreeNode("Brake Command Response Params:")) {
		ImGui::Text("Higher torque locks wheels quicker - Value strongly related to wheel MOI");
		if (ImGui::InputFloat("Max Response:", &brake_max)) {			
			gVehicle.mBaseParams.brakeResponseParams->maxResponse = brake_max;
		}

		if (ImGui::TreeNode("Wheel Response Multipliers")) {
			ImGui::Text("These are multipliers, 0.0 means the brake is disabled, 1.0 is full brake power");
			ImGui::Text("Handbrake = 0 front, 1 back");

			if (ImGui::SliderFloat("Front Right Wheel:", &brake_response_multip[0], 0.f, 1.f)) {
				gVehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers[0] = brake_response_multip[0];
			}
			if (ImGui::SliderFloat("Front Left Wheel:", &brake_response_multip[1], 0.f, 1.f)) {
				gVehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers[1] = brake_response_multip[1];
			}
			if (ImGui::SliderFloat("Rear Right Wheel:", &brake_response_multip[2], 0.f, 1.f)) {
				gVehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers[2] = brake_response_multip[2];
			}
			if (ImGui::SliderFloat("Read Left Wheel:", &brake_response_multip[3], 0.f, 1.f)) {
				gVehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers[3] = brake_response_multip[3];
			}

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	// Steer Commands
	if (ImGui::TreeNode("Steer Response Params:")) {
		ImGui::Text("Yaw angle of the wheel in radians when steering wheel is at full lock");
		ImGui::Text("30 - 90 Degrees is a good starting point (0.523 - 1.57)");
		ImGui::Text("Large angles at large speeds will cause vehicle to lose traction");
		ImGui::Text("To avoid traction loss, filter the steer angle passed to the car at run-time to generate smaller steer angles are larger speeds");
		if (ImGui::InputFloat("Max Response:", &steer_max_response)) {
			gVehicle.mBaseParams.steerResponseParams.maxResponse = steer_max_response;
		}

		if (ImGui::TreeNode("Wheel Response Multiplier")) {
			ImGui::Text("0.0 disables steering with that wheel");

			if (ImGui::SliderFloat("Front Right Wheel", &steer_multiplier[0], 0.f, 1.f)) {
				gVehicle.mBaseParams.steerResponseParams.wheelResponseMultipliers[0] = steer_multiplier[0];
			}
			if (ImGui::SliderFloat("Front Left Wheel", &steer_multiplier[1], 0.f, 1.f)) {
				gVehicle.mBaseParams.steerResponseParams.wheelResponseMultipliers[1] = steer_multiplier[1];
			}
			if (ImGui::SliderFloat("Back Right Wheel", &steer_multiplier[2], 0.f, 1.f)) {
				gVehicle.mBaseParams.steerResponseParams.wheelResponseMultipliers[2] = steer_multiplier[2];
			}
			if (ImGui::SliderFloat("Back Left Wheel", &steer_multiplier[3], 0.f, 1.f)) {
				gVehicle.mBaseParams.steerResponseParams.wheelResponseMultipliers[3] = steer_multiplier[3];
			}

			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	// Ackerman Angle (Angle of the wheels on the y axis when turning)
	if (ImGui::TreeNode("Ackerman Angle:")) {
		ImGui::Text("Ackerman Angle allows for better cornering.");
		ImGui::Text("Steers the left and right wheels differently around corners");

		ImGui::Separator();

		ImGui::Text("Distance between center of the axle and references axle (rear axle)");
		if (ImGui::InputFloat("Wheel Base", &ack_wheel_base)) {
			gVehicle.mBaseParams.ackermannParams->wheelBase = ack_wheel_base;
		}

		ImGui::Text("Distance between the wheels and the axle being corrected");
		if (ImGui::InputFloat("Track Width", &ack_track_width)) {
			gVehicle.mBaseParams.ackermannParams->trackWidth = ack_track_width;
		}

		ImGui::Text("Strength of the correction from 0 is no correction, 1 is full correction");
		if (ImGui::SliderFloat("Strength", &ack_strength, 0.f, 1.f)) {
			gVehicle.mBaseParams.ackermannParams->strength = ack_strength;
		}
		
		ImGui::TreePop();
	}


	// TODO:: Make this dynamic based on number of wheels - probably unecessary for this project
	// Wheel Params
	if (ImGui::TreeNode("Wheel Params:")) {
		ImGui::Text("Caution: Changing these values may break the simulation");
		// Front Right Wheel
		if (ImGui::TreeNode("Front Right Wheel:")) {
			ImGui::Text("Distance between the center of the wheel and the outside of the rim");
			ImGui::Text("Ideally exported from 3D modeler");
			if (ImGui::InputFloat("Radius", &wheel_radius[0])) {
				gVehicle.mBaseParams.wheelParams[0].radius = wheel_radius[0];
			}
			if (ImGui::InputFloat("Half Width", &wheel_half_width[0])) {
				gVehicle.mBaseParams.wheelParams[0].halfWidth = wheel_half_width[0];
			}
			ImGui::Text("Combined mass of wheel + tire. Typical mass 20-80kg");
			if (ImGui::InputFloat("Mass", &wheel_mass[0])) {
				gVehicle.mBaseParams.wheelParams[0].mass = wheel_mass[0];
			}
			ImGui::Text("Large MOI values make it harder for the wheel to rotate around its axis");
			ImGui::Text("Large MOI = less wheelspin when high acceleration");
			ImGui::Text("Formula: moi = 0.5 * mass * radius^2");
			if (ImGui::InputFloat("MOI", & wheel_moi[0])) {
				gVehicle.mBaseParams.wheelParams[0].moi = wheel_moi[0];
			}
			ImGui::Text("Dampening is how quickly a spinning wheel comes to rest");
			ImGui::Text("Higher values make wheel come to rest faster (short amount of time)");
			ImGui::Text("Sensible Range 0.25-2, avoid value of 0");
			if (ImGui::InputFloat("Dampening", &wheel_dampening[0])) {
				gVehicle.mBaseParams.wheelParams[0].dampingRate = wheel_dampening[0];
			}


			ImGui::TreePop();
		}

		// Front Left Wheel
		if (ImGui::TreeNode("Front Left Wheel:")) {
			ImGui::Text("Distance between the center of the wheel and the outside of the rim");
			ImGui::Text("Ideally exported from 3D modeler");
			if (ImGui::InputFloat("Radius", &wheel_radius[1])) {
				gVehicle.mBaseParams.wheelParams[1].radius = wheel_radius[1];
			}
			if (ImGui::InputFloat("Half Width", &wheel_half_width[1])) {
				gVehicle.mBaseParams.wheelParams[1].halfWidth = wheel_half_width[1];
			}
			ImGui::Text("Combined mass of wheel + tire. Typical mass 20-80kg");
			if (ImGui::InputFloat("Mass", &wheel_mass[1])) {
				gVehicle.mBaseParams.wheelParams[1].mass = wheel_mass[1];
			}
			ImGui::Text("Large MOI values make it harder for the wheel to rotate around its axis");
			ImGui::Text("Large MOI = less wheelspin when high acceleration");
			ImGui::Text("Formula: moi = 0.5 * mass * radius^2");
			if (ImGui::InputFloat("MOI", &wheel_moi[1])) {
				gVehicle.mBaseParams.wheelParams[1].moi = wheel_moi[1];
			}
			ImGui::Text("Dampening is how quickly a spinning wheel comes to rest");
			ImGui::Text("Higher values make wheel come to rest faster (short amount of time)");
			ImGui::Text("Sensible Range 0.25-2, avoid value of 0");
			if (ImGui::InputFloat("Dampening", &wheel_dampening[1])) {
				gVehicle.mBaseParams.wheelParams[1].dampingRate = wheel_dampening[1];
			}


			ImGui::TreePop();
		}

		// Rear Right Wheel
		if (ImGui::TreeNode("Rear Right Wheel:")) {
			ImGui::Text("Distance between the center of the wheel and the outside of the rim");
			ImGui::Text("Ideally exported from 3D modeler");
			if (ImGui::InputFloat("Radius", &wheel_radius[2])) {
				gVehicle.mBaseParams.wheelParams[2].radius = wheel_radius[2];
			}
			if (ImGui::InputFloat("Half Width", &wheel_half_width[0])) {
				gVehicle.mBaseParams.wheelParams[2].halfWidth = wheel_half_width[2];
			}
			ImGui::Text("Combined mass of wheel + tire. Typical mass 20-80kg");
			if (ImGui::InputFloat("Mass", &wheel_mass[2])) {
				gVehicle.mBaseParams.wheelParams[2].mass = wheel_mass[2];
			}
			ImGui::Text("Large MOI values make it harder for the wheel to rotate around its axis");
			ImGui::Text("Large MOI = less wheelspin when high acceleration");
			ImGui::Text("Formula: moi = 0.5 * mass * radius^2");
			if (ImGui::InputFloat("MOI", &wheel_moi[2])) {
				gVehicle.mBaseParams.wheelParams[2].moi = wheel_moi[2];
			}
			ImGui::Text("Dampening is how quickly a spinning wheel comes to rest");
			ImGui::Text("Higher values make wheel come to rest faster (short amount of time)");
			ImGui::Text("Sensible Range 0.25-2, avoid value of 0");
			if (ImGui::InputFloat("Dampening", &wheel_dampening[2])) {
				gVehicle.mBaseParams.wheelParams[2].dampingRate = wheel_dampening[2];
			}


			ImGui::TreePop();
		}

		// Rear Left Wheel
		if (ImGui::TreeNode("Rear Left Wheel:")) {
			ImGui::Text("Distance between the center of the wheel and the outside of the rim");
			ImGui::Text("Ideally exported from 3D modeler");
			if (ImGui::InputFloat("Radius", &wheel_radius[3])) {
				gVehicle.mBaseParams.wheelParams[3].radius = wheel_radius[3];
			}
			if (ImGui::InputFloat("Half Width", &wheel_half_width[3])) {
				gVehicle.mBaseParams.wheelParams[3].halfWidth = wheel_half_width[3];
			}
			ImGui::Text("Combined mass of wheel + tire. Typical mass 20-80kg");
			if (ImGui::InputFloat("Mass", &wheel_mass[3])) {
				gVehicle.mBaseParams.wheelParams[3].mass = wheel_mass[3];
			}
			ImGui::Text("Large MOI values make it harder for the wheel to rotate around its axis");
			ImGui::Text("Large MOI = less wheelspin when high acceleration");
			ImGui::Text("Formula: moi = 0.5 * mass * radius^2");
			if (ImGui::InputFloat("MOI", &wheel_moi[3])) {
				gVehicle.mBaseParams.wheelParams[3].moi = wheel_moi[3];
			}
			ImGui::Text("Dampening is how quickly a spinning wheel comes to rest");
			ImGui::Text("Higher values make wheel come to rest faster (short amount of time)");
			ImGui::Text("Sensible Range 0.25-2, avoid value of 0");
			if (ImGui::InputFloat("Dampening", &wheel_dampening[3])) {
				gVehicle.mBaseParams.wheelParams[3].dampingRate = wheel_dampening[3];
			}


			ImGui::TreePop();
		}

		ImGui::TreePop();
	}


	ImGui::End();
}

void reloadVehicleJSON() {
	ImGui::Begin("Vehicle Serialization");



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