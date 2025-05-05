#pragma once

#include "pch.h"

namespace game_engine {
	class Timestep
	{
	public:
		Timestep(std::chrono::duration<float, std::chrono::seconds::period> time);

		std::chrono::duration<float, std::chrono::seconds::period> get_seconds() const;
		std::chrono::duration<float, std::chrono::milliseconds::period> get_milliseconds() const;

		void print() const;
		float count() const;

		Timestep& operator=(const std::chrono::duration<float, std::chrono::seconds::period>& timestep);
		Timestep& operator-=(const Timestep & timestep);
		Timestep operator-(const Timestep& timestep) const;
		Timestep& operator+=(const Timestep& timestep);
		Timestep operator+(const Timestep& timestep) const;
		bool operator<=(const std::chrono::duration<float, std::chrono::seconds::period>& other) const;

		operator float() const { return timestep.count(); };
		glm::vec3 operator*(const glm::vec3& other) const
		{
			auto ts = timestep.count();
			return glm::vec3(ts * other.x, ts * other.y, ts * other.z);
		}
	private:
		std::chrono::duration<float, std::chrono::seconds::period> timestep;
	};
}