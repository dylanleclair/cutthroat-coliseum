#pragma once

#include "../Car.h"
#include "../AICar.h"

void init_sound_system();

void update_sounds(Car& car, AICar& opponent, bool playSounds);

void handle_fmod_error();

void play_beep_sound();
