#include "SDL.h"

struct FramerateCounter {
	static const int m_capacity = 60;
	FramerateCounter() {
		m_queue.reserve(m_capacity + 1);
	}

	uint32_t m_prev = 0;
	std::vector<uint32_t> m_queue;

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
		m_queue.emplace(m_queue.begin(), delta);

		if (m_queue.size() > m_capacity) m_queue.pop_back();

		uint32_t sum = 0;
		for (const auto& t : m_queue) sum += t;
		return 1000 * m_queue.size() / sum;
	}
};