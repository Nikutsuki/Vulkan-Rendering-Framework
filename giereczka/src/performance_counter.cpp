#include "performance_counter.h"

void game_engine::PerformanceCounter::start()
{
	auto now = std::chrono::high_resolution_clock::now();
	current_time = std::chrono::duration<float, std::chrono::seconds::period>(now.time_since_epoch()).count();
	is_running = true;
	frame_times.clear();
	frame_times.resize(buffer_size);
}

void game_engine::PerformanceCounter::stop()
{
}

void game_engine::PerformanceCounter::run(float frame_time)
{
	if (is_running)
	{
		if (frame_times.size() >= buffer_size)
		{
			frame_times.pop_front();
		}
		frame_times.push_back(delta_time);

		delta_time = frame_time;

		current_time += delta_time;
	}
}
