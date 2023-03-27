#pragma once

#include "core/ecs.h"
#include "../../systems/components.h"
#include "Car.h"

void setupCarVFX(ecs::Scene& mainScene, Guid _ID);
void updateCarVFX(ecs::Scene mainScene, float _dt);

void tireTrackImgui();
