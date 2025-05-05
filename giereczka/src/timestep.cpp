#include "timestep.h"

game_engine::Timestep::Timestep(std::chrono::duration<float, std::chrono::seconds::period> time) : timestep(time)
{
}

std::chrono::duration<float, std::chrono::seconds::period> game_engine::Timestep::get_seconds() const
{
	return timestep;
}

std::chrono::duration<float, std::chrono::milliseconds::period> game_engine::Timestep::get_milliseconds() const
{
	return std::chrono::duration_cast<std::chrono::duration<float, std::chrono::milliseconds::period>>(timestep);
}

void game_engine::Timestep::print() const
{
	auto milliseconds = get_milliseconds();
	std::cout << milliseconds.count() << "ms\n";
	auto seconds = get_seconds();
	std::cout << seconds.count() << "s\n";
}

float game_engine::Timestep::count() const
{
	return timestep.count();
}

game_engine::Timestep& game_engine::Timestep::operator=(const std::chrono::duration<float, std::chrono::seconds::period>& timestep)
{
	this->timestep = timestep;
	return *this;
}

game_engine::Timestep& game_engine::Timestep::operator-=(const Timestep& timestep)
{
	this->timestep -= timestep.timestep;
	return *this;
}

game_engine::Timestep game_engine::Timestep::operator-(const Timestep& timestep) const
{
	return this->timestep - timestep.timestep;
}

game_engine::Timestep& game_engine::Timestep::operator+=(const Timestep& timestep)
{
	this->timestep += timestep.timestep;
	return *this;
}

game_engine::Timestep game_engine::Timestep::operator+(const Timestep& timestep) const
{
	return this->timestep + timestep.timestep;
}

bool game_engine::Timestep::operator<=(const std::chrono::duration<float, std::chrono::seconds::period>& other) const
{
	return (this->timestep.count() - other.count()) <= 0.f;
}
