#pragma once
// TODO(beau): turn this file into our general time handling system
#include <cassert>
#include "utils/Time.h"

struct FramerateCounter {
	static const int m_capacity = 60;
	FramerateCounter() {
		m_time_queue_ms.reserve(m_capacity + 1);
		m_rate_queue.reserve(m_capacity + 1);
	}

	// TODO(beau): use proper queue data structures when I care
	std::vector<float> m_time_queue_ms; // in milliseconds
	std::vector<float> m_rate_queue; // in frames per second / hz

	// call only once per frame!!
	void update(Timestep frametime) {
		m_time_queue_ms.emplace(m_time_queue_ms.begin(), frametime.getMilliseconds());

		if (m_time_queue_ms.size() > m_capacity) {
            m_time_queue_ms.pop_back();
            m_rate_queue.pop_back();
        }

		float sum = 0;
		for (const auto t : m_time_queue_ms) sum += t;
        float curr_framerate = 1000 * m_time_queue_ms.size() / sum;

		m_rate_queue.emplace(m_rate_queue.begin(), curr_framerate);
        assert(m_rate_queue.size() == m_time_queue_ms.size());
	}

	// call only after update() is called on the same frame!!!
	float framerate() { return m_rate_queue[0]; }
};
