#include "ImGuiTuneables.h"
#include <iostream>

// Initializes variables
void baseVariablesInit(EngineDriveVehicle &m_Vehicle) {
	all_wheels = true; // sets the boolean flag to affect all wheels

	rigid_mass = m_Vehicle.mBaseParams.rigidBodyParams.mass;
	rigid_MOI = m_Vehicle.mBaseParams.rigidBodyParams.moi;

	brake_max = m_Vehicle.mBaseParams.brakeResponseParams->maxResponse;
	brake_response_multip = m_Vehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers;

	steer_max_response = m_Vehicle.mBaseParams.steerResponseParams.maxResponse;
	steer_multiplier = m_Vehicle.mBaseParams.steerResponseParams.wheelResponseMultipliers;

	number_of_wheels = 4;
	// Grabs values from each wheel and assigns it to variables for this file to edit
	for (int i = 0; i < number_of_wheels; i++) {
		wheel_radius[i] = m_Vehicle.mBaseParams.wheelParams[i].radius;
		wheel_half_width[i] = m_Vehicle.mBaseParams.wheelParams[i].halfWidth;
		wheel_mass[i] = m_Vehicle.mBaseParams.wheelParams[i].mass;
		wheel_moi[i] = m_Vehicle.mBaseParams.wheelParams[i].moi;
		wheel_dampening[i] = m_Vehicle.mBaseParams.wheelParams[i].dampingRate;

		// Suspension Force Params
		sus_sprung_mass[i] = m_Vehicle.mBaseParams.suspensionForceParams[i].sprungMass;
		sus_stiffness[i] = m_Vehicle.mBaseParams.suspensionForceParams[i].stiffness;
		sus_dampening[i] = m_Vehicle.mBaseParams.suspensionForceParams[i].damping;
		dampening_ratio[i] = sus_dampening[i] / (2 * sqrt(sus_stiffness[i] * sus_sprung_mass[i]));
	}

	// Unused wheel ID's supposed be a pair [0,1] means front wheels, [2,3] means back wheels
	// The pair is a designation of which wheel pair is the ackerman angle
	//ack_wheel_ids = m_Vehicle.mBaseParams.ackermannParams->wheelIds;
	ack_wheel_base = m_Vehicle.mBaseParams.ackermannParams->wheelBase;
	ack_track_width = m_Vehicle.mBaseParams.ackermannParams->trackWidth;
	ack_strength = m_Vehicle.mBaseParams.ackermannParams->strength;

	// Suspension Params (How much suspension squishes and in which direction)
	//sus_attach = m_Vehicle.mBaseParams.suspensionParams->suspensionAttachment;
	//sus_travel_dir = m_Vehicle.mBaseParams.suspensionParams->suspensionTravelDir;
	sus_travel_dist = m_Vehicle.mBaseParams.suspensionParams->suspensionTravelDist;
	//sus_wheel_attach = m_Vehicle.mBaseParams.suspensionParams->wheelAttachment;*/

	// Suspension State Calculations
	//sus_jounce = m_Vehicle.mBaseParams.suspensionStateCalculationParams.suspensionJounceCalculationType;
	sus_limit_xpvel = m_Vehicle.mBaseParams.suspensionStateCalculationParams.limitSuspensionExpansionVelocity;

	// Suspension Compliance (Way too complex parameters, not needed for this project)
	// TODO:: Need to initate header values
	//m_Vehicle.mBaseParams.suspensionComplianceParams->wheelToeAngle;
	//m_Vehicle.mBaseParams.suspensionComplianceParams->wheelCamberAngle;
	//m_Vehicle.mBaseParams.suspensionComplianceParams->suspForceAppPoint;
	//m_Vehicle.mBaseParams.suspensionComplianceParams->tireForceAppPoint;	
}

// Initalizes variables for the engine drive model
void engineVariablesInit(EngineDriveVehicle &m_Vehicle) {
	// Engine
	eng_moi = m_Vehicle.mEngineDriveParams.engineParams.moi;
	eng_torque = m_Vehicle.mEngineDriveParams.engineParams.peakTorque;
	eng_torque_curve = m_Vehicle.mEngineDriveParams.engineParams.torqueCurve;
	eng_idle_omega = m_Vehicle.mEngineDriveParams.engineParams.idleOmega;
	eng_max_omega = m_Vehicle.mEngineDriveParams.engineParams.maxOmega;
	eng_damp_full = m_Vehicle.mEngineDriveParams.engineParams.dampingRateFullThrottle;
	eng_damp_engage = m_Vehicle.mEngineDriveParams.engineParams.dampingRateZeroThrottleClutchEngaged;
	eng_damp_disengage = m_Vehicle.mEngineDriveParams.engineParams.dampingRateZeroThrottleClutchDisengaged;

	// Gearing
	// Index of the neutral gear, we are using default gearing for this project so I am commenting it out
	// if you uncomment this don't forget to uncomment the header
	//gear_neutral = m_Vehicle.mEngineDriveParams.gearBoxParams.neutralGear;	
	gear_final = m_Vehicle.mEngineDriveParams.gearBoxParams.finalRatio;
	gear_switch_time = m_Vehicle.mEngineDriveParams.gearBoxParams.switchTime;

	//gear_nb = m_Vehicle.mEngineDriveParams.gearBoxParams.nbRatios; //number of gears, we will use default value for this project
	for (int i = 0; i < 7; i++) {
		gear_ratios[i] = m_Vehicle.mEngineDriveParams.gearBoxParams.ratios[i];

		// Autobox
		auto_up[i] = m_Vehicle.mEngineDriveParams.autoboxParams.upRatios[i];
		auto_down[i] = m_Vehicle.mEngineDriveParams.autoboxParams.downRatios[i];
	}
	
	auto_latency = m_Vehicle.mEngineDriveParams.autoboxParams.latency;
}

// Possible other tuning to add - these are not necessary for the game:
// Axle (Might be important if custom vehicle), (changes the size, length ect.. of axle)
// Anti Roll Bar (not used in the base.json ?)
// Tire Force (Slip Params) (Complex parameters possibly not needed for this project)
// 
// Clutch Command Response
// Differentials

// Function to print different ratio outputs for the dampening portion of the suspension code
void dampeningRatioPrint(int i) {
	ImGui::Text("Dampening ratio formula = ratio = dampening / [2 * sqrt(stiffness * sprungMass)]");
	if (dampening_ratio[i] > 1.0f) {
		ImGui::Text("Dampening Ratio: %f", dampening_ratio[i]);
		ImGui::Text("Over dampened - try not to exceed a ratio of 1.2");
		ImGui::Text("Handling may be twitchy beyond this rate");
	}
	else if (dampening_ratio[i] == 1.0f) {
		ImGui::Text("Dampening Ratio: %f", dampening_ratio[i]);
		ImGui::Text("Critical dampened - This is a good thing");
	}
	else {
		ImGui::Text("Dampening Ratio: %f", dampening_ratio[i]);
		ImGui::Text("Under dampened - try not to have a ratio lower than 0.8");
		ImGui::Text("Handling may be sluggish beyond this rate");
	}
}

void vehicleTuning(EngineDriveVehicle &m_Vehicle) {
	ImGui::Begin("Vehicle Tuning");

	// Rigid Body params
	if (ImGui::TreeNode("Rigid Body:")) {

		if (ImGui::InputFloat("Mass: (in kg)", &rigid_mass)) {
			m_Vehicle.mBaseParams.rigidBodyParams.mass = rigid_mass;
		}

		if (ImGui::TreeNode("MOI: (affects turning and rolling)")) {
			ImGui::Text("Higher values make it harder to induce rotation speed from tire, and suspension forces");
			ImGui::Text("Physical values: x= (L^2 + H^2)*M/12, y=(W^2+L^2)*M/12, z=(H^2+W^2)*M/12");
			if (ImGui::InputFloat("MOI x: ", &rigid_MOI.x)) {
				m_Vehicle.mBaseParams.rigidBodyParams.moi.x = rigid_MOI.x;
			}
			if (ImGui::InputFloat("MOI y: ", &rigid_MOI.y)) {
				m_Vehicle.mBaseParams.rigidBodyParams.moi.y = rigid_MOI.y;
			}
			if (ImGui::InputFloat("MOI z: ", &rigid_MOI.z)) {
				m_Vehicle.mBaseParams.rigidBodyParams.moi.z = rigid_MOI.z;
			}
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	// Brake Commands
	if (ImGui::TreeNode("Brake Command Response Params:")) {
		ImGui::Text("Higher torque locks wheels quicker - Value strongly related to wheel MOI");
		if (ImGui::InputFloat("Max Response:", &brake_max)) {			
			m_Vehicle.mBaseParams.brakeResponseParams->maxResponse = brake_max;
		}

		if (ImGui::TreeNode("Wheel Response Multipliers")) {
			ImGui::Text("These are multipliers, 0.0 means the brake is disabled, 1.0 is full brake power");
			ImGui::Text("Handbrake = 0 front, 1 back");

			if (ImGui::SliderFloat("Front Right Wheel:", &brake_response_multip[0], 0.f, 1.f)) {
				m_Vehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers[0] = brake_response_multip[0];
			}
			if (ImGui::SliderFloat("Front Left Wheel:", &brake_response_multip[1], 0.f, 1.f)) {
				m_Vehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers[1] = brake_response_multip[1];
			}
			if (ImGui::SliderFloat("Rear Right Wheel:", &brake_response_multip[2], 0.f, 1.f)) {
				m_Vehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers[2] = brake_response_multip[2];
			}
			if (ImGui::SliderFloat("Read Left Wheel:", &brake_response_multip[3], 0.f, 1.f)) {
				m_Vehicle.mBaseParams.brakeResponseParams->wheelResponseMultipliers[3] = brake_response_multip[3];
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
			m_Vehicle.mBaseParams.steerResponseParams.maxResponse = steer_max_response;
		}

		if (ImGui::TreeNode("Wheel Response Multiplier")) {
			ImGui::Text("0.0 disables steering with that wheel");

			if (ImGui::SliderFloat("Front Right Wheel", &steer_multiplier[0], 0.f, 1.f)) {
				m_Vehicle.mBaseParams.steerResponseParams.wheelResponseMultipliers[0] = steer_multiplier[0];
			}
			if (ImGui::SliderFloat("Front Left Wheel", &steer_multiplier[1], 0.f, 1.f)) {
				m_Vehicle.mBaseParams.steerResponseParams.wheelResponseMultipliers[1] = steer_multiplier[1];
			}
			if (ImGui::SliderFloat("Back Right Wheel", &steer_multiplier[2], 0.f, 1.f)) {
				m_Vehicle.mBaseParams.steerResponseParams.wheelResponseMultipliers[2] = steer_multiplier[2];
			}
			if (ImGui::SliderFloat("Back Left Wheel", &steer_multiplier[3], 0.f, 1.f)) {
				m_Vehicle.mBaseParams.steerResponseParams.wheelResponseMultipliers[3] = steer_multiplier[3];
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
			m_Vehicle.mBaseParams.ackermannParams->wheelBase = ack_wheel_base;
		}

		ImGui::Text("Distance between the wheels and the axle being corrected");
		if (ImGui::InputFloat("Track Width", &ack_track_width)) {
			m_Vehicle.mBaseParams.ackermannParams->trackWidth = ack_track_width;
		}

		ImGui::Text("Strength of the correction from 0 is no correction, 1 is full correction");
		if (ImGui::SliderFloat("Strength", &ack_strength, 0.f, 1.f)) {
			m_Vehicle.mBaseParams.ackermannParams->strength = ack_strength;
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
				m_Vehicle.mBaseParams.wheelParams[0].radius = wheel_radius[0];
			}
			if (ImGui::InputFloat("Half Width", &wheel_half_width[0])) {
				m_Vehicle.mBaseParams.wheelParams[0].halfWidth = wheel_half_width[0];
			}
			ImGui::Text("Combined mass of wheel + tire. Typical mass 20-80kg");
			if (ImGui::InputFloat("Mass", &wheel_mass[0])) {
				m_Vehicle.mBaseParams.wheelParams[0].mass = wheel_mass[0];
			}
			ImGui::Text("Large MOI values make it harder for the wheel to rotate around its axis");
			ImGui::Text("Large MOI = less wheelspin when high acceleration");
			ImGui::Text("Formula: moi = 0.5 * mass * radius^2");
			if (ImGui::InputFloat("MOI", & wheel_moi[0])) {
				m_Vehicle.mBaseParams.wheelParams[0].moi = wheel_moi[0];
			}
			ImGui::Text("Dampening is how quickly a spinning wheel comes to rest");
			ImGui::Text("Higher values make wheel come to rest faster (short amount of time)");
			ImGui::Text("Sensible Range 0.25-2, avoid value of 0");
			if (ImGui::InputFloat("Dampening", &wheel_dampening[0])) {
				m_Vehicle.mBaseParams.wheelParams[0].dampingRate = wheel_dampening[0];
			}


			ImGui::TreePop();
		}

		// Front Left Wheel
		if (ImGui::TreeNode("Front Left Wheel:")) {
			ImGui::Text("Distance between the center of the wheel and the outside of the rim");
			ImGui::Text("Ideally exported from 3D modeler");
			if (ImGui::InputFloat("Radius", &wheel_radius[1])) {
				m_Vehicle.mBaseParams.wheelParams[1].radius = wheel_radius[1];
			}
			if (ImGui::InputFloat("Half Width", &wheel_half_width[1])) {
				m_Vehicle.mBaseParams.wheelParams[1].halfWidth = wheel_half_width[1];
			}
			ImGui::Text("Combined mass of wheel + tire. Typical mass 20-80kg");
			if (ImGui::InputFloat("Mass", &wheel_mass[1])) {
				m_Vehicle.mBaseParams.wheelParams[1].mass = wheel_mass[1];
			}
			ImGui::Text("Large MOI values make it harder for the wheel to rotate around its axis");
			ImGui::Text("Large MOI = less wheelspin when high acceleration");
			ImGui::Text("Formula: moi = 0.5 * mass * radius^2");
			if (ImGui::InputFloat("MOI", &wheel_moi[1])) {
				m_Vehicle.mBaseParams.wheelParams[1].moi = wheel_moi[1];
			}
			ImGui::Text("Dampening is how quickly a spinning wheel comes to rest");
			ImGui::Text("Higher values make wheel come to rest faster (short amount of time)");
			ImGui::Text("Sensible Range 0.25-2, avoid value of 0");
			if (ImGui::InputFloat("Dampening", &wheel_dampening[1])) {
				m_Vehicle.mBaseParams.wheelParams[1].dampingRate = wheel_dampening[1];
			}


			ImGui::TreePop();
		}

		// Rear Right Wheel
		if (ImGui::TreeNode("Rear Right Wheel:")) {
			ImGui::Text("Distance between the center of the wheel and the outside of the rim");
			ImGui::Text("Ideally exported from 3D modeler");
			if (ImGui::InputFloat("Radius", &wheel_radius[2])) {
				m_Vehicle.mBaseParams.wheelParams[2].radius = wheel_radius[2];
			}
			if (ImGui::InputFloat("Half Width", &wheel_half_width[0])) {
				m_Vehicle.mBaseParams.wheelParams[2].halfWidth = wheel_half_width[2];
			}
			ImGui::Text("Combined mass of wheel + tire. Typical mass 20-80kg");
			if (ImGui::InputFloat("Mass", &wheel_mass[2])) {
				m_Vehicle.mBaseParams.wheelParams[2].mass = wheel_mass[2];
			}
			ImGui::Text("Large MOI values make it harder for the wheel to rotate around its axis");
			ImGui::Text("Large MOI = less wheelspin when high acceleration");
			ImGui::Text("Formula: moi = 0.5 * mass * radius^2");
			if (ImGui::InputFloat("MOI", &wheel_moi[2])) {
				m_Vehicle.mBaseParams.wheelParams[2].moi = wheel_moi[2];
			}
			ImGui::Text("Dampening is how quickly a spinning wheel comes to rest");
			ImGui::Text("Higher values make wheel come to rest faster (short amount of time)");
			ImGui::Text("Sensible Range 0.25-2, avoid value of 0");
			if (ImGui::InputFloat("Dampening", &wheel_dampening[2])) {
				m_Vehicle.mBaseParams.wheelParams[2].dampingRate = wheel_dampening[2];
			}


			ImGui::TreePop();
		}

		// Rear Left Wheel
		if (ImGui::TreeNode("Rear Left Wheel:")) {
			ImGui::Text("Distance between the center of the wheel and the outside of the rim");
			ImGui::Text("Ideally exported from 3D modeler");
			if (ImGui::InputFloat("Radius", &wheel_radius[3])) {
				m_Vehicle.mBaseParams.wheelParams[3].radius = wheel_radius[3];
			}
			if (ImGui::InputFloat("Half Width", &wheel_half_width[3])) {
				m_Vehicle.mBaseParams.wheelParams[3].halfWidth = wheel_half_width[3];
			}
			ImGui::Text("Combined mass of wheel + tire. Typical mass 20-80kg");
			if (ImGui::InputFloat("Mass", &wheel_mass[3])) {
				m_Vehicle.mBaseParams.wheelParams[3].mass = wheel_mass[3];
			}
			ImGui::Text("Large MOI values make it harder for the wheel to rotate around its axis");
			ImGui::Text("Large MOI = less wheelspin when high acceleration");
			ImGui::Text("Formula: moi = 0.5 * mass * radius^2");
			if (ImGui::InputFloat("MOI", &wheel_moi[3])) {
				m_Vehicle.mBaseParams.wheelParams[3].moi = wheel_moi[3];
			}
			ImGui::Text("Dampening is how quickly a spinning wheel comes to rest");
			ImGui::Text("Higher values make wheel come to rest faster (short amount of time)");
			ImGui::Text("Sensible Range 0.25-2, avoid value of 0");
			if (ImGui::InputFloat("Dampening", &wheel_dampening[3])) {
				m_Vehicle.mBaseParams.wheelParams[3].dampingRate = wheel_dampening[3];
			}


			ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	// Suspension (Currently setup for only 4 wheels)
	if (ImGui::TreeNode("Suspension:")) {
		ImGui::Text("Distance between wheel at maximum droop, and maximum compression");
		if (ImGui::InputFloat("Travel Direction:", &sus_travel_dist)) {
			m_Vehicle.mBaseParams.suspensionParams->suspensionTravelDist = sus_travel_dist;
		}

		ImGui::Text("Setting to TRUE leads to greater simulation integrity (takes longer for spring to expand)");
		ImGui::Text("This may be undersirable as the vehicle will spend more time not touching the ground");
		ImGui::Text("This leads to the player losing control of the vehicle more often");
		if (ImGui::Checkbox("Limit Expansion Velocity", &sus_limit_xpvel)) {
			m_Vehicle.mBaseParams.suspensionStateCalculationParams.limitSuspensionExpansionVelocity = sus_limit_xpvel;
		}

		ImGui::Separator();
		ImGui::Separator();
		ImGui::Checkbox("Uniform values for all wheels", &all_wheels);

		if (all_wheels) {
						
			ImGui::Separator();
			ImGui::Text("Mass supported by the springs, if center of mass is in middle, these should");
			ImGui::Text("all be 1/4 of the mass of the rigid body");
			if (ImGui::InputFloat("Sprung Mass", &sus_sprung_mass[0])) {
				for (int i = 0; i < 4; i++) {
					m_Vehicle.mBaseParams.suspensionForceParams[i].sprungMass = sus_sprung_mass[i]; // recalculate dampening ratio
					dampening_ratio[i] = sus_dampening[i] / (2 * sqrt(sus_stiffness[i] * sus_sprung_mass[i]));
				}
			}

			ImGui::Separator();
			ImGui::Text("Higher Stiffness = harder to turn vehicle");
			ImGui::Text("Higher Stiffness = Bumpier ride");
			if (ImGui::InputFloat("Stiffness", &sus_stiffness[0])) {
				for (int i = 0; i < 4; i++) {
					m_Vehicle.mBaseParams.suspensionForceParams[i].sprungMass = sus_sprung_mass[i]; // recalculate dampening ratio
					m_Vehicle.mBaseParams.suspensionForceParams[i].stiffness = sus_stiffness[i];
				}
			}

			ImGui::Text("Dampening ratio formula = ratio = dampening / [2 * sqrt(stiffness * sprungMass)]");
			if (dampening_ratio[0] > 1.0f) {
				ImGui::Text("Dampening Ratio: %f", dampening_ratio[0]);
				ImGui::Text("Over dampened - try not to exceed a ratio of 1.2");
				ImGui::Text("Handling may be twitchy beyond this rate");
			}
			else if (dampening_ratio[0] == 1.0f) {
				ImGui::Text("Dampening Ratio: %f", dampening_ratio[0]);
				ImGui::Text("Critical dampened - This is a good thing");
			}
			else {
				ImGui::Text("Dampening Ratio: %f", dampening_ratio[0]);
				ImGui::Text("Under dampened - try not to have a ratio lower than 0.8");
				ImGui::Text("Handling may be sluggish beyond this rate");
			}

			if (ImGui::InputFloat("Dampening", &sus_dampening[0])) {
				for (int i = 0; i < 4; i++) {
					m_Vehicle.mBaseParams.suspensionForceParams[i].sprungMass = sus_sprung_mass[i]; // recalculate dampening ratio
					m_Vehicle.mBaseParams.suspensionForceParams[i].damping = sus_dampening[i];
				}
			}
		}
		else {	
			ImGui::Text("Mass supported by the springs, if center of mass is in middle, these should");
			ImGui::Text("all be 1/4 of the mass of the rigid body");

				if (ImGui::InputFloat("Sprung Mass - Front Right", &sus_sprung_mass[0])) {
					m_Vehicle.mBaseParams.suspensionForceParams[0].sprungMass = sus_sprung_mass[0];
				}
				if (ImGui::InputFloat("Sprung Mass - Front Left", &sus_sprung_mass[1])) {
					m_Vehicle.mBaseParams.suspensionForceParams[1].sprungMass = sus_sprung_mass[1];
				}
				if (ImGui::InputFloat("Sprung Mass - Rear Right", &sus_sprung_mass[2])) {
					m_Vehicle.mBaseParams.suspensionForceParams[2].sprungMass = sus_sprung_mass[2];
				}
				if (ImGui::InputFloat("Sprung Mass - Rear Left", &sus_sprung_mass[3])) {
					m_Vehicle.mBaseParams.suspensionForceParams[3].sprungMass = sus_sprung_mass[3];
				}

				ImGui::Separator();
				ImGui::Text("Higher Stiffness = harder to turn vehicle");
				ImGui::Text("Higher Stiffness = Bumpier ride");

				if (ImGui::InputFloat("Stiffness - Front Right", &sus_stiffness[0])) {
					m_Vehicle.mBaseParams.suspensionForceParams[0].stiffness = sus_stiffness[0];
				}
				if (ImGui::InputFloat("Stiffness - Front Left", &sus_stiffness[1])) {
					m_Vehicle.mBaseParams.suspensionForceParams[1].stiffness = sus_stiffness[1];
				}
				if (ImGui::InputFloat("Stiffness - Rear Right", &sus_stiffness[2])) {
					m_Vehicle.mBaseParams.suspensionForceParams[2].stiffness = sus_stiffness[2];
				}
				if (ImGui::InputFloat("Stiffness - Rear Left", &sus_stiffness[3])) {
					m_Vehicle.mBaseParams.suspensionForceParams[3].stiffness = sus_stiffness[3];
				}

				dampeningRatioPrint(0);
				if (ImGui::InputFloat("Dampening - Front Right", &sus_dampening[0])) {
					m_Vehicle.mBaseParams.suspensionForceParams[0].sprungMass = sus_sprung_mass[0]; // recalculate dampening ratio
					m_Vehicle.mBaseParams.suspensionForceParams[0].damping = sus_dampening[0];
				}
				dampeningRatioPrint(1);
				if (ImGui::InputFloat("Dampening - Front Left", &sus_dampening[1])) {
					m_Vehicle.mBaseParams.suspensionForceParams[1].sprungMass = sus_sprung_mass[1]; // recalculate dampening ratio
					m_Vehicle.mBaseParams.suspensionForceParams[1].damping = sus_dampening[1];
				}
				dampeningRatioPrint(2);
				if (ImGui::InputFloat("Dampening - Rear Right", &sus_dampening[2])) {
					m_Vehicle.mBaseParams.suspensionForceParams[2].sprungMass = sus_sprung_mass[2]; // recalculate dampening ratio
					m_Vehicle.mBaseParams.suspensionForceParams[2].damping = sus_dampening[2];
				}
				dampeningRatioPrint(3);
				if (ImGui::InputFloat("Dampening - Rear Left", &sus_dampening[3])) {
					m_Vehicle.mBaseParams.suspensionForceParams[3].sprungMass = sus_sprung_mass[3]; // recalculate dampening ratio
					m_Vehicle.mBaseParams.suspensionForceParams[3].damping = sus_dampening[3];
				}
		}


		ImGui::TreePop();
	}


	ImGui::End();
}

void engineTuning(EngineDriveVehicle &m_Vehicle) {
	ImGui::Begin("Engine Tuning");

	if (ImGui::TreeNode("Engine Params:")) {

		ImGui::Text("Larger values make it harder to accelerate (1.0 is a good starting value)");
		if (ImGui::InputFloat("MOI", &eng_moi)) {
			m_Vehicle.mEngineDriveParams.engineParams.moi = eng_moi;
		}

		ImGui::Separator();
		ImGui::Text("Max Torque - 600 good starting value - expressed in Newton Metres");
		if (ImGui::InputFloat("Peak Torque", &eng_torque)) {
			m_Vehicle.mEngineDriveParams.engineParams.peakTorque = eng_torque;
		}

		// CURRENTLY BUGGY NOT WORKING
		// TODO:: The .xVals .yVals calls are not working as intended (they are grabbing all values)
		// May need to change the header type 
		// 
		//ImGui::Separator();
		//ImGui::Text("Torque Curve");
		//ImGui::Text("X values are normalized engine speed");
		//ImGui::Text("Y values are multiplier range (0,1)");
		//if (ImGui::InputFloat("x", &eng_torque_curve.xVals[0])) {
		//	m_Vehicle.mEngineDriveParams.engineParams.torqueCurve.xVals[0] = eng_torque_curve.xVals[0];
		//}
		//ImGui::SameLine();
		//if (ImGui::InputFloat("y", &eng_torque_curve.yVals[0])) {
		//	m_Vehicle.mEngineDriveParams.engineParams.torqueCurve.yVals[0] = eng_torque_curve.yVals[0];
		//}


		//if (ImGui::InputFloat("x", &eng_torque_curve.xVals[1])) {
		//	m_Vehicle.mEngineDriveParams.engineParams.torqueCurve.xVals[1] = eng_torque_curve.xVals[1];
		//}
		//ImGui::SameLine();
		//if (ImGui::InputFloat("y", &eng_torque_curve.yVals[1])) {
		//	m_Vehicle.mEngineDriveParams.engineParams.torqueCurve.yVals[1] = eng_torque_curve.yVals[1];
		//}


		//if (ImGui::InputFloat("x", &eng_torque_curve.xVals[2])) {
		//	m_Vehicle.mEngineDriveParams.engineParams.torqueCurve.xVals[2] = eng_torque_curve.xVals[2];
		//}
		//ImGui::SameLine();
		//if (ImGui::InputFloat("y", &eng_torque_curve.yVals[2])) {
		//	m_Vehicle.mEngineDriveParams.engineParams.torqueCurve.yVals[2] = eng_torque_curve.yVals[2];
		//}



		ImGui::Separator();
		ImGui::Text("Lowest rotation speed of the engine, expressed in radians per second");
		if (ImGui::InputFloat("Idle Omega", &eng_idle_omega)) {
			m_Vehicle.mEngineDriveParams.engineParams.idleOmega = eng_idle_omega;
		}

		ImGui::Separator();
		ImGui::Text("Maximum rotation speed of the engine, expressed in radians per second");
		if (ImGui::InputFloat("Max Omega", &eng_max_omega)) {
			m_Vehicle.mEngineDriveParams.engineParams.maxOmega = eng_max_omega;
		}

		ImGui::Separator();
		ImGui::Text("(0.25 to 3) are good values, 0 will make simulation unstable");
		ImGui::Text("Dampening rates for the following situations: ");

		if (ImGui::InputFloat("Full Throttle", &eng_damp_full)) {
			m_Vehicle.mEngineDriveParams.engineParams.dampingRateFullThrottle = eng_damp_full;
		}

		if (ImGui::InputFloat("Clutch Engaged", &eng_damp_engage)) {
			m_Vehicle.mEngineDriveParams.engineParams.dampingRateZeroThrottleClutchEngaged = eng_damp_engage;
		}

		if (ImGui::InputFloat("Clutch Disengaged", &eng_damp_disengage)) {
			m_Vehicle.mEngineDriveParams.engineParams.dampingRateZeroThrottleClutchDisengaged = eng_damp_disengage;
		}
				

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Gearbox:")) {
		ImGui::Text("Current gear is multipled by final ratio");
		ImGui::Text("Fast way to edit all gear ratios without having to edit each one");
		ImGui::Text("Typical value is around 4");
		if (ImGui::InputFloat("Final Ratio", &gear_final)) {
			m_Vehicle.mEngineDriveParams.gearBoxParams.finalRatio = gear_final;
		}

		ImGui::Separator();
		ImGui::Text("If you want to edit each gear ratio manually");
		ImGui::Text("First gear is reverse, second gear is neutral (must be 0)");
		if (ImGui::InputFloat("Reverse Gear", &gear_ratios[0])) {
			m_Vehicle.mEngineDriveParams.gearBoxParams.ratios[0] = gear_ratios[0];
		}
		if (ImGui::InputFloat("Neutral Gear", &gear_ratios[1])) {
			m_Vehicle.mEngineDriveParams.gearBoxParams.ratios[1] = gear_ratios[1];
		}
		if (ImGui::InputFloat("1st Gear", &gear_ratios[2])) {
			m_Vehicle.mEngineDriveParams.gearBoxParams.ratios[2] = gear_ratios[2];
		}
		if (ImGui::InputFloat("2nd Gear", &gear_ratios[3])) {
			m_Vehicle.mEngineDriveParams.gearBoxParams.ratios[3] = gear_ratios[3];
		}
		if (ImGui::InputFloat("3rd Gear", &gear_ratios[4])) {
			m_Vehicle.mEngineDriveParams.gearBoxParams.ratios[4] = gear_ratios[4];
		}
		if (ImGui::InputFloat("4th Gear", &gear_ratios[5])) {
			m_Vehicle.mEngineDriveParams.gearBoxParams.ratios[5] = gear_ratios[5];
		}
		if (ImGui::InputFloat("5th Gear", &gear_ratios[6])) {
			m_Vehicle.mEngineDriveParams.gearBoxParams.ratios[6] = gear_ratios[6];
		}

		ImGui::Separator();
		ImGui::Text("How long it takes in seconds for gears to switch");
		if (ImGui::InputFloat("Switch Time", &gear_switch_time)) {
			m_Vehicle.mEngineDriveParams.gearBoxParams.switchTime = gear_switch_time;
		}		
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Autobox:")) {
		ImGui::Text("How long (in seconds) until the next gear change happens");
		ImGui::Text("You want to make sure this is higher than your gear switch time");
		ImGui::Text("Current gear switch time: %f", gear_switch_time);
		if (ImGui::InputFloat("Lantency", &auto_latency)) {
			m_Vehicle.mEngineDriveParams.autoboxParams.latency = auto_latency;
		}

		ImGui::Separator();
		ImGui::Text("Autobox will initate a gear change if the ratio of the engine rotation speed");
		ImGui::Text("is greater than the ratio in the table, the inverse for shifting down");
		ImGui::Text("Up From || ");
		ImGui::SameLine();
		ImGui::Text("Down From");
		ImGui::Text("Rev");
		ImGui::SameLine();
		if (ImGui::InputFloat("", &auto_up[0])) {
			m_Vehicle.mEngineDriveParams.autoboxParams.upRatios[0] = auto_up[0];
		}
		ImGui::SameLine();
		if (ImGui::InputFloat("", &auto_down[0])) {
			m_Vehicle.mEngineDriveParams.autoboxParams.downRatios[0] = auto_down[0];
		}
		ImGui::Text("Neu");
		ImGui::SameLine();
		if (ImGui::InputFloat("", &auto_up[1])) {
			m_Vehicle.mEngineDriveParams.autoboxParams.upRatios[1] = auto_up[1];
		}
		ImGui::SameLine();
		if (ImGui::InputFloat("", &auto_down[1])) {
			m_Vehicle.mEngineDriveParams.autoboxParams.downRatios[1] = auto_down[1];
		}
		ImGui::Text("1st");
		ImGui::SameLine();
		if (ImGui::InputFloat("", &auto_up[2])) {
			m_Vehicle.mEngineDriveParams.autoboxParams.upRatios[2] = auto_up[2];
		}
		ImGui::SameLine();
		if (ImGui::InputFloat("", &auto_down[2])) {
			m_Vehicle.mEngineDriveParams.autoboxParams.downRatios[2] = auto_down[2];
		}
		ImGui::Text("2nd");
		ImGui::SameLine();
		if (ImGui::InputFloat("", &auto_up[3])) {
			m_Vehicle.mEngineDriveParams.autoboxParams.upRatios[3] = auto_up[3];
		}
		ImGui::SameLine();
		if (ImGui::InputFloat("", &auto_down[3])) {
			m_Vehicle.mEngineDriveParams.autoboxParams.downRatios[3] = auto_down[3];
		}
		ImGui::Text("3rd");
		ImGui::SameLine();
		if (ImGui::InputFloat("", &auto_up[4])) {
			m_Vehicle.mEngineDriveParams.autoboxParams.upRatios[4] = auto_up[4];
		}
		ImGui::SameLine();
		if (ImGui::InputFloat("", &auto_down[4])) {
			m_Vehicle.mEngineDriveParams.autoboxParams.downRatios[4] = auto_down[4];
		}
		ImGui::Text("4th");
		ImGui::SameLine();
		if (ImGui::InputFloat("", &auto_up[5])) {
			m_Vehicle.mEngineDriveParams.autoboxParams.upRatios[5] = auto_up[5];
		}
		ImGui::SameLine();
		if (ImGui::InputFloat("", &auto_down[5])) {
			m_Vehicle.mEngineDriveParams.autoboxParams.downRatios[5] = auto_down[5];
		}
		ImGui::Text("5th");
		ImGui::SameLine();
		if (ImGui::InputFloat("", &auto_up[6])) {
			m_Vehicle.mEngineDriveParams.autoboxParams.upRatios[6] = auto_up[6];
		}
		ImGui::SameLine();
		if (ImGui::InputFloat("", &auto_down[6])) {
			m_Vehicle.mEngineDriveParams.autoboxParams.downRatios[6] = auto_down[6];
		}

		ImGui::TreePop();
	}

	

	ImGui::End();
}

void reloadVehicleJSON(EngineDriveVehicle &m_Vehicle) {
	ImGui::Begin("Vehicle Serialization");



	// Reads the JSON in the assets vehicle data
	// If first param set to m_VehicleDataPath it goes to the out folder path instead
	if (ImGui::Button("Read Base.Json")) {
		readBaseParamsFromJsonFile("../../../../assets/vehicledata", "Base.json", m_Vehicle.mBaseParams);
	}

	// Reads the JSON in the assets vehicle data
	// If first param set to m_VehicleDataPath it goes to the out folder path instead
	if (ImGui::Button("Read EngineDrive.Json")) {
		readEngineDrivetrainParamsFromJsonFile("../../../../assets/vehicledata", "EngineDrive.json", m_Vehicle.mEngineDriveParams);
	}
	
	ImGui::End();
}