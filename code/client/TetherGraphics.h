#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "systems/components.h"
#include "Car.h"

std::vector<float> distFromTarget(TransformComponent _origin_object, TransformComponent _target_object);
int findSmallestDistanceIndex(std::vector<float> _array);
void updateTetherGraphic(TransformComponent& car_trans, std::vector<TransformComponent> tether_points,
	Car& testCar, TransformComponent& tether_transform);