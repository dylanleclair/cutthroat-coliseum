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
	FMOD::Sound* drivesound = NULL;
    FMOD::Sound* idlesound = NULL;
	FMOD::Sound* brakesound = NULL;
	FMOD::Sound* collisionsound = NULL;
} soundsystem;

void SoundUpdater::Initialize() {}

void SoundUpdater::Initialize(ecs::Scene &scene)
{
    CarSoundEmitter emitter = {};

    for (auto id : ecs::EntitiesInScene<Car>(scene))
    {
        scene.AddComponent(id, emitter);
    }
}

void SoundUpdater::Update(ecs::Scene &scene, float deltaTime)
{
    for (auto id : ecs::EntitiesInScene<Car>(scene))
    {
        
        auto & car = scene.GetComponent<Car>(id);
        
        if (car.m_driverType == DriverType::COMPUTER)
        {
            auto & channel = scene.GetComponent<CarSoundEmitter>(id);

            auto & engine = channel.enginechannel;
            auto & idle = channel.idlechannel;
            auto & brake = channel.brakechannel;
            // auto & collision = channel.collisionchannel;

            bool isPlaying = false;
            engine->isPlaying(&isPlaying);
            if (!isPlaying && is_car_throttling(car))
            {
                result = soundsystem.system->playSound(soundsystem.drivesound, 0, false, &engine);
                handle_fmod_error();

                brake->stop();
                idle->stop();
            }

            isPlaying = false;
            idle->isPlaying(&isPlaying);
            if ( !isPlaying && !is_car_throttling(car) && !is_car_braking(car) ) {
                result = soundsystem.system->playSound(soundsystem.idlesound, 0, false, &idle);
                handle_fmod_error();

                brake->stop();
                engine->stop();
            }

            isPlaying = false;
            brake->isPlaying(&isPlaying);
            if (!isPlaying && is_car_braking(car) && car.m_Vehicle.mBaseState.roadGeomStates->hitState) {

                result = soundsystem.system->playSound(soundsystem.brakesound, 0, false, &brake);
                handle_fmod_error();

                // also turn off the engine sound
                engine->stop();
                idle->stop();
            }

            auto body     = car.getVehicleRigidBody();
            auto pose     = body->getGlobalPose();
            auto velocity = body->getLinearVelocity();
            auto fmod_vel = px_to_fmod_vec3(velocity);
            auto position = px_to_fmod_vec3(pose.p);

            engine->set3DAttributes(&position, &fmod_vel);
            brake->set3DAttributes(&position, &fmod_vel);
            idle->set3DAttributes(&position, &fmod_vel);
            // collision->set3DAttributes(&position, &fmod_vel);
        } else {
                auto & car = scene.GetComponent<Car>(id);
            auto & channel = scene.GetComponent<CarSoundEmitter>(id);

            auto & engine = channel.enginechannel;
            auto & idle = channel.idlechannel;
            auto & brake = channel.brakechannel;
            // auto & collision = channel.collisionchannel;

            {
                const float playervolume = 0.15f;
                engine->setVolume(playervolume);
                brake->setVolume(playervolume);
                idle->setVolume(playervolume);
            }

            bool isPlaying = false;
            engine->isPlaying(&isPlaying);
            if (!isPlaying && is_car_throttling(car))
            {
                result = soundsystem.system->playSound(soundsystem.drivesound, 0, false, &engine);
                handle_fmod_error();

                brake->stop();
                idle->stop();
            }

            isPlaying = false;
            idle->isPlaying(&isPlaying);
            if ( !isPlaying && !is_car_throttling(car) && !is_car_braking(car) ) {
                result = soundsystem.system->playSound(soundsystem.idlesound, 0, false, &idle);
                handle_fmod_error();

                brake->stop();
                engine->stop();
            }

            isPlaying = false;
            brake->isPlaying(&isPlaying);
            if (!isPlaying && is_car_braking(car) && car.m_Vehicle.mBaseState.roadGeomStates->hitState) {

                result = soundsystem.system->playSound(soundsystem.brakesound, 0, false, &brake);
                handle_fmod_error();

                // also turn off the engine sound
                engine->stop();
                idle->stop();
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
            idle->set3DAttributes(&position, &fmod_vel);
            // collision->set3DAttributes(&position, &fmod_vel);
        }
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

	result = soundsystem.system->createSound("audio/beep.ogg", FMOD_3D, 0, &soundsystem.beepsound);
	handle_fmod_error();

	result = soundsystem.system->createSound("audio/brake-6315.mp3", FMOD_3D, 0, &soundsystem.brakesound);
	handle_fmod_error();

	result = soundsystem.system->createSound("audio/engine_drive.mp3", FMOD_3D, 0, &soundsystem.drivesound);
	handle_fmod_error();

	result = soundsystem.system->createSound("audio/engine_idle.mp3", FMOD_3D, 0, &soundsystem.idlesound);
	handle_fmod_error();

	result = soundsystem.system->createSound("audio/dark_pit_theme.mp3", FMOD_LOOP_NORMAL, 0, &soundsystem.musicsound);
	handle_fmod_error();

    result = soundsystem.system->playSound(soundsystem.musicsound, 0, false, &soundsystem.musicchannel);
	handle_fmod_error();

    // make music quiet
    result = soundsystem.musicchannel->setVolume(0.1f);
    handle_fmod_error();

    // fix attenuation

    result = soundsystem.brakesound->set3DMinMaxDistance(1.f, MAX_SOUND_DISTANCE);
    handle_fmod_error();

    result = soundsystem.drivesound->set3DMinMaxDistance(1.f, MAX_SOUND_DISTANCE);
    handle_fmod_error();

    result = soundsystem.idlesound->set3DMinMaxDistance(1.f, MAX_SOUND_DISTANCE);
    handle_fmod_error();

	// TODO: collision sound
	//result = soundsystem.system->createSound("audio/beep.ogg", FMOD_3D, 0, &soundsystem.beepsound);
	//handle_fmod_error();
}

