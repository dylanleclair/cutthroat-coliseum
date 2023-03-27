#include "SoundSystem.h"

#include "fmod.hpp"
#include "fmod_errors.h"

#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

#include "../Input.h"

#include "core/ecs.h"

#include <cstdio>
#include <cstdlib>

const float MAX_SOUND_DISTANCE = 500000.f;

const float MIN_SOUND_THRESHHOLD = 0.1f;

FMOD_RESULT result;

void handle_fmod_error() {
	if (result == FMOD_OK) return;

	printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
    exit(-1);
}

FMOD_VECTOR px_to_fmod_vec3(PxVec3 v) {
	return FMOD_VECTOR{
		v.x,
		v.y,
		v.z
	};
}

bool is_car_throttling(const Car& car) {
	const auto& commands = car.m_Vehicle.mCommandState;

	return commands.throttle > MIN_SOUND_THRESHHOLD;
}

bool is_car_braking(const Car& car) {
	const auto& commands = car.m_Vehicle.mCommandState;

	return commands.brakes[0] > MIN_SOUND_THRESHHOLD;
}

struct SoundSystem {
	FMOD::System* system = NULL;

    // music things
	FMOD::Sound* musicsound = NULL;
	FMOD::Channel* musicchannel = NULL;

    // car sounds
	FMOD::Sound* beepsound = NULL;
	FMOD::Sound* enginesound = NULL;
	FMOD::Sound* brakesound = NULL;
	FMOD::Sound* collisionsound = NULL;

    // player channels
	FMOD::Channel* playerenginechannel = NULL;
	FMOD::Channel* playerbrakechannel = NULL;
	FMOD::Channel* playercollisionchannel = NULL;

    // opponent channels
	FMOD::Channel* opponentenginechannel = NULL;
	FMOD::Channel* opponentbrakechannel = NULL;
	FMOD::Channel* opponentcollisionchannel = NULL;
} soundsystem;

void SoundUpdater::Initialize() {}

void SoundUpdater::Initialize(ecs::Scene &scene)
{
    CarSoundEmitter emitter = {};

    for (auto id : ecs::EntitiesInScene<AICar>(scene))
    {
        scene.AddComponent(id, emitter);
    }

   for (auto id : ecs::EntitiesInScene<Car>(scene))
    {
        scene.AddComponent(id, emitter);
    }
}

void SoundUpdater::Update(ecs::Scene &scene, float deltaTime)
{
    // NOTE(beau): this is almost a complete duplilcate of the player update below,
    // however its not so easy to factor everything out believe it or not
    for (auto id : ecs::EntitiesInScene<AICar>(scene))
    {
        auto & car = scene.GetComponent<AICar>(id);
        auto & channel = scene.GetComponent<CarSoundEmitter>(id);

        auto & engine = channel.enginechannel;
        auto & brake = channel.brakechannel;
        // auto & collision = channel.collisionchannel;

        bool isPlaying = false;
        engine->isPlaying(&isPlaying);
        if (!isPlaying && is_car_throttling(car))
        {
            result = soundsystem.system->playSound(soundsystem.enginesound, 0, false, &engine);
            handle_fmod_error();

            brake->stop();
        }

        isPlaying = false;
        brake->isPlaying(&isPlaying);
        if (!isPlaying && is_car_braking(car)) {

            result = soundsystem.system->playSound(soundsystem.brakesound, 0, false, &brake);
            handle_fmod_error();

            // also turn off the engine sound
            engine->stop();
        }

        auto body     = car.getVehicleRigidBody();
        auto pose     = body->getGlobalPose();
        auto velocity = body->getLinearVelocity();
        auto fmod_vel = px_to_fmod_vec3(velocity);
        auto position = px_to_fmod_vec3(pose.p);

        engine->set3DAttributes(&position, &fmod_vel);
        brake->set3DAttributes(&position, &fmod_vel);
        // collision->set3DAttributes(&position, &fmod_vel);
    }
    for (auto id : ecs::EntitiesInScene<Car>(scene))
    {
        auto & car = scene.GetComponent<Car>(id);
        auto & channel = scene.GetComponent<CarSoundEmitter>(id);

        auto & engine = channel.enginechannel;
        auto & brake = channel.brakechannel;
        // auto & collision = channel.collisionchannel;

        bool isPlaying = false;
        engine->isPlaying(&isPlaying);
        if (!isPlaying && is_car_throttling(car))
        {
            // result = soundsystem.system->playSound(soundsystem.enginesound, 0, false, &engine);
            // handle_fmod_error();

            brake->stop();
        }

        isPlaying = false;
        brake->isPlaying(&isPlaying);
        if (!isPlaying && is_car_braking(car)) {

            // result = soundsystem.system->playSound(soundsystem.brakesound, 0, false, &brake);
            // handle_fmod_error();

            // also turn off the engine sound
            engine->stop();
        }

        auto body     = car.getVehicleRigidBody();
        auto pose     = body->getGlobalPose();
        auto velocity = body->getLinearVelocity();
        auto fmod_vel = px_to_fmod_vec3(velocity);
        auto position = px_to_fmod_vec3(pose.p);

        // set listener position
        PxMat33 rotation(pose.q);
        auto forward = px_to_fmod_vec3(rotation * PxVec3{0,0,1});
        auto up = px_to_fmod_vec3(rotation * PxVec3{0,1,0});
        result = soundsystem.system->set3DListenerAttributes(0, &position, &fmod_vel, &forward, &up);
        handle_fmod_error();

        engine->set3DAttributes(&position, &fmod_vel);
        brake->set3DAttributes(&position, &fmod_vel);
        // collision->set3DAttributes(&position, &fmod_vel);
    }

	result = soundsystem.system->update();
	handle_fmod_error();
}



void init_sound_system() {
	result = FMOD::System_Create(&soundsystem.system);      // Create the main system object.
	handle_fmod_error();

	result = soundsystem.system->init(512, FMOD_INIT_3D_RIGHTHANDED, 0);    // Initialize FMOD.
	handle_fmod_error();

    // create sounds

	// result = soundsystem.system->createSound("audio/beep.ogg", FMOD_3D, 0, &soundsystem.beepsound);
	// handle_fmod_error();

	result = soundsystem.system->createSound("audio/brake-6315.mp3", FMOD_3D, 0, &soundsystem.brakesound);
	handle_fmod_error();

	result = soundsystem.system->createSound("audio/bus-engine-idling-26992.mp3", FMOD_3D, 0, &soundsystem.enginesound);
	handle_fmod_error();

	result = soundsystem.system->createSound("audio/dark_pit_theme.mp3", FMOD_LOOP_NORMAL, 0, &soundsystem.musicsound);
	handle_fmod_error();

    result = soundsystem.system->playSound(soundsystem.musicsound, 0, false, &soundsystem.musicchannel);
	handle_fmod_error();

    // fix attenuation

    result = soundsystem.brakesound->set3DMinMaxDistance(1.f, MAX_SOUND_DISTANCE);
    handle_fmod_error();

    result = soundsystem.enginesound->set3DMinMaxDistance(1.f, MAX_SOUND_DISTANCE);
    handle_fmod_error();


	// TODO: collision sound
	//result = soundsystem.system->createSound("audio/beep.ogg", FMOD_3D, 0, &soundsystem.beepsound);
	//handle_fmod_error();
}

void update_sounds(Car& player, AICar& opponent, bool playSounds) {
	// PLAYER: play engine sound if we're throttling and not already playing on the channel
	bool isPlaying = false;
	soundsystem.playerenginechannel->isPlaying(&isPlaying);
	if (!isPlaying && is_car_throttling(player)) {

		result = soundsystem.system->playSound(soundsystem.enginesound, 0, false, &soundsystem.playerenginechannel);
		handle_fmod_error();

		// also turn off brake sound
		soundsystem.playerbrakechannel->stop();
	}
	soundsystem.playerenginechannel->setVolume(0.5f);
	soundsystem.musicchannel->setVolume(0.3f);

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

	if (!playSounds) {
		soundsystem.playerbrakechannel->stop();
		soundsystem.playerenginechannel->stop();
		soundsystem.playercollisionchannel->stop();

		soundsystem.opponentbrakechannel->stop();
		soundsystem.opponentenginechannel->stop();
		soundsystem.opponentcollisionchannel->stop();
	}

	result = soundsystem.system->update();
	handle_fmod_error();
}

void play_beep_sound() {
	puts("pressed the sound key");
	result = soundsystem.system->playSound(soundsystem.beepsound, 0, false, &soundsystem.playerenginechannel);
	handle_fmod_error();
}
