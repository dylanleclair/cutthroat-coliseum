// TODO(beau): turn this file into our general time handling system
#include <cassert>
#include "SDL.h"

struct FramerateCounter {
	static const int m_capacity = 60;
	FramerateCounter() {
		m_time_queue.reserve(m_capacity + 1);
		m_rate_queue.reserve(m_capacity + 1);
	}

	uint32_t m_prev = 0;
	// TODO(beau): use proper queue data structures when I care
	std::vector<float> m_time_queue; // in milliseconds
	std::vector<float> m_rate_queue; // in frames per second / hz

	// for framerate counter
	uint32_t deltatime() {
		uint32_t curr = SDL_GetTicks();
		auto delta = curr - m_prev;
		m_prev = curr;
		return delta;
	}

	// call only once per frame!!
	uint32_t framerate() {
		uint32_t delta = deltatime();

		// HACK(beau): clamp lowest possible frametime to 1 ms
		// otherwise we pass 0 to physx simulate which is no good
		// this is an issue if your gpu has vsync turned off and
		// your framerate gets above 1000 which causes SDL_GetTicks()
		// to return the same value as last frame as another
		// millisecond hasn't passed yet meaning the deltatime
		// since last frame would be 0
		delta = delta ? delta : 1;

		assert(delta > 0);
		m_time_queue.emplace(m_time_queue.begin(), (float) delta);

		if (m_time_queue.size() > m_capacity) {
            m_time_queue.pop_back();
            m_rate_queue.pop_back();
        }

		uint32_t sum = 0;
		for (const auto& t : m_time_queue) sum += t;
        uint32_t result = 1000 * m_time_queue.size() / sum;

		m_rate_queue.emplace(m_rate_queue.begin(), (float) result);
        assert(m_rate_queue.size() == m_time_queue.size());

        return result;
	}
};
