#pragma once

#include <chrono>
#include <deque>

namespace game_engine {
	class PerformanceCounter {
	public:
		PerformanceCounter() = default;
		~PerformanceCounter() = default;
		void start();
		void stop();

		void run(float frame_time);

		float get_delta_time() const { return delta_time; }
		float get_current_time() const { return current_time; }
		float get_fps() const { return 1.f / delta_time; }

		std::deque<float>& get_frame_times() { return frame_times; }

		bool get_is_running() const { return is_running; }
	private:
		float current_time = 0.f;
		float delta_time = 0.f;
		bool is_running = false;

		std::deque<float> frame_times;
		int buffer_size = 1000;
	};
}