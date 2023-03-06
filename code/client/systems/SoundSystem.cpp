#include "SoundSystem.h"

#include "fmod.hpp"
#include "fmod_errors.h"

#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

#include "../Input.h"

#include <cstdio>
#include <cstdlib>

FMOD_RESULT result;

struct SoundSystem {
	FMOD::System* system = NULL;

	FMOD::Sound* beepsound = NULL;
	FMOD::Sound* enginesound = NULL;
	FMOD::Sound* brakesound = NULL;
	FMOD::Sound* collisionsound = NULL;

	FMOD::Channel* playerenginechannel = NULL;
	FMOD::Channel* playerbrakechannel = NULL;
	FMOD::Channel* playercollisionchannel = NULL;

	FMOD::Channel* opponentenginechannel = NULL;
	FMOD::Channel* opponentbrakechannel = NULL;
	FMOD::Channel* opponentcollisionchannel = NULL;
} soundsystem;

void handle_fmod_error() {
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		exit(-1);
	}
}

void init_sound_system() {
	result = FMOD::System_Create(&soundsystem.system);      // Create the main system object.
	handle_fmod_error();

	result = soundsystem.system->init(512, FMOD_INIT_3D_RIGHTHANDED, 0);    // Initialize FMOD.
	handle_fmod_error();

	result = soundsystem.system->createSound("audio/beep.ogg", FMOD_3D, 0, &soundsystem.beepsound);
	handle_fmod_error();

	result = soundsystem.system->createSound("audio/brake-6315.mp3", FMOD_3D, 0, &soundsystem.brakesound);
	handle_fmod_error();

	result = soundsystem.system->createSound("audio/bus-engine-idling-26992.mp3", FMOD_3D, 0, &soundsystem.enginesound);
	handle_fmod_error();

	// TODO: collision sound
	//result = soundsystem.system->createSound("audio/beep.ogg", FMOD_3D, 0, &soundsystem.beepsound);
	//handle_fmod_error();
}

FMOD_VECTOR px_to_fmod_vec3(PxVec3 v) {
	return FMOD_VECTOR{
		v.x,
		v.y,
		v.z
	};
}

const float MIN_SOUND_THRESHHOLD = 0.1f;

bool is_car_throttling(const Car& car) {
	const auto& commands = car.m_Vehicle.mCommandState;

	return commands.throttle > MIN_SOUND_THRESHHOLD;
}

bool is_car_braking(const Car& car) {
	const auto& commands = car.m_Vehicle.mCommandState;

	return commands.brakes[0] > MIN_SOUND_THRESHHOLD;
}

void update_sounds(Car& player, AICar& opponent) {
	// PLAYER: play engine sound if we're throttling and not already playing on the channel
	bool isPlaying = false;
	soundsystem.playerenginechannel->isPlaying(&isPlaying);
	if (!isPlaying && is_car_throttling(player)) {

		result = soundsystem.system->playSound(soundsystem.enginesound, 0, false, &soundsystem.playerenginechannel);
		handle_fmod_error();

		// also turn off brake sound
		soundsystem.playerbrakechannel->stop();
	}

	// PLAYER: play brake sound if we're throttling and not already playing on the channel
	isPlaying = false;
	soundsystem.playerbrakechannel->isPlaying(&isPlaying);
	if (!isPlaying && is_car_braking(player)) {

		result = soundsystem.system->playSound(soundsystem.brakesound, 0, false, &soundsystem.playerbrakechannel);
		handle_fmod_error();

		// also turn off the engine sound
		soundsystem.playerenginechannel->stop();
	}

	// OPPONENT: play engine sound if we're throttling and not already playing on the channel
	isPlaying = false;
	soundsystem.opponentenginechannel->isPlaying(&isPlaying);
	if (!isPlaying && is_car_throttling(opponent)) {

		result = soundsystem.system->playSound(soundsystem.enginesound, 0, false, &soundsystem.opponentenginechannel);
		handle_fmod_error();

		// also turn off brake sound
		soundsystem.opponentbrakechannel->stop();
	}

	// OPPONENT: play brake sound if we're throttling and not already playing on the channel
	isPlaying = false;
	soundsystem.playerbrakechannel->isPlaying(&isPlaying);
	if (!isPlaying && is_car_braking(opponent)) {

		result = soundsystem.system->playSound(soundsystem.brakesound, 0, false, &soundsystem.opponentbrakechannel);
		handle_fmod_error();

		// also turn off the engine sound
		soundsystem.opponentenginechannel->stop();
	}

	// set position and velocity of player sound channels, and everything for listener on the player
	{
		auto playerbody = player.getVehicleRigidBody();
		auto playerpose = playerbody->getGlobalPose();
		auto playerveloc = playerbody->getLinearVelocity();
		auto playerposition = px_to_fmod_vec3(playerpose.p);

		// set player listener position
		PxMat33 playerrotation(playerpose.q);
		auto forward = px_to_fmod_vec3(playerrotation * PxVec3{ 0, 0, 1 });
		auto up = px_to_fmod_vec3(playerrotation * PxVec3{ 0, 1, 0 });
		result = soundsystem.system->set3DListenerAttributes(0, &playerposition, &px_to_fmod_vec3(playerveloc), &forward, &up);
		handle_fmod_error();

		soundsystem.playerenginechannel->set3DAttributes(&playerposition, &px_to_fmod_vec3(playerveloc));
		soundsystem.playerbrakechannel->set3DAttributes(&playerposition, &px_to_fmod_vec3(playerveloc));
		soundsystem.playercollisionchannel->set3DAttributes(&playerposition, &px_to_fmod_vec3(playerveloc));
	}

	// set position and velocity of opponent sound channels
	{
		auto opponentbody = opponent.getVehicleRigidBody();
		auto opponentpose = opponentbody->getGlobalPose();
		auto opponentveloc = px_to_fmod_vec3(opponentbody->getLinearVelocity());
		auto opponentposition = px_to_fmod_vec3(opponentpose.p);

		soundsystem.opponentenginechannel->set3DAttributes(&opponentposition, &opponentveloc);
		soundsystem.opponentbrakechannel->set3DAttributes(&opponentposition, &opponentveloc);
		soundsystem.opponentcollisionchannel->set3DAttributes(&opponentposition, &opponentveloc);
	}


	result = soundsystem.system->update();
	handle_fmod_error();
}

void play_beep_sound() {
	puts("pressed the sound key");
	result = soundsystem.system->playSound(soundsystem.beepsound, 0, false, &soundsystem.playerenginechannel);
	handle_fmod_error();
}