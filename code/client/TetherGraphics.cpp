#include "TetherGraphics.h"


// Returns the x and y components of distances from the target object
// returns the magnitude / distance from to origin to target
std::vector<float> distFromTarget(TransformComponent _origin_object, TransformComponent _target_object) {
	std::vector<float> values;

	float x_diff = _origin_object.getTranslation().x - _target_object.getTranslation().x;
	float z_diff = _origin_object.getTranslation().z - _target_object.getTranslation().z;

	values.push_back(x_diff);
	values.push_back(z_diff);

	values.push_back(sqrt(pow(abs(x_diff), 2) + pow(abs(z_diff), 2)));

	return values;
}

// Is given an array, and returns the index of the smallest element in the array
// THE ARRAY SHOULD NOT BE SORTED
int findSmallestDistanceIndex(std::vector<float> _array) {
	float value = _array[0];
	int index = 0;
	for (int i = 0; i < _array.size(); i++) {
		float val_compare = _array[i];

		if (val_compare < value) {
			index = i;
		}
	}

	return index;
}

void updateTetherGraphic(TransformComponent& car_trans, std::vector<TransformComponent> tether_points,
	Car& testCar, TransformComponent& tether_transform) {
	int closestIndex = 0;
	std::vector<float> x_distances;
	std::vector<float> z_distances;
	std::vector<float> total_distances;

	x_distances.clear();
	z_distances.clear();
	total_distances.clear();

	for (int i = 0; i < tether_points.size(); i++) {
		std::vector<float> p_distance = distFromTarget(car_trans, tether_points[i]);
		x_distances.push_back(p_distance[0]);
		z_distances.push_back(p_distance[1]);
		total_distances.push_back(p_distance[2]);
	}

	// Sets the closest tether point of the car to the closest tether point 
	closestIndex = findSmallestDistanceIndex(total_distances);
	testCar.setClosestTetherPoint(tether_points[closestIndex].getTranslation());

	float tether_angle = atan(x_distances[closestIndex] / z_distances[closestIndex]);
	// Puts the tether's origin at the tether point
	tether_transform.setPosition(glm::vec3(tether_points[closestIndex].getTranslation().x, 1.5f, tether_points[closestIndex].getTranslation().z));
	// Sets the scale of the tether on the x axis in accordance to the distance of the car to the tether point
	tether_transform.setScale(glm::vec3(total_distances[closestIndex], 1.f, 1.f));
	// Rotates the tether to track the car
	if (z_distances[closestIndex] < 0) {
		tether_transform.setRotation(glm::vec3(0, 1, 0), tether_angle + (M_PI / 2.f));
	}
	else {
		tether_transform.setRotation(glm::vec3(0, 1, 0), tether_angle + (3 * M_PI / 2.f));
	}
}
