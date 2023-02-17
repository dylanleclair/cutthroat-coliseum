#pragma once

struct Timestep {
	Timestep(float delta_ms = 0.f) : milliseconds(delta_ms) {};

	float milliseconds;

	float getSeconds() { return milliseconds / 1000.f; };
	float getMilliseconds() { return milliseconds; };
};