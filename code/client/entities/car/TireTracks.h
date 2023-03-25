#pragma once

#include "core/ecs.h"
#include "../../systems/components.h"
#include "Car.h"

void setupTireTrackVisuals(ecs::Scene& mainScene, int number_of_vehicles);
void setupPreviousStates(int number_of_vehicles);
void TireTracks(Car& car, std::vector<Guid>& AIGuids, ecs::Scene mainScene);

