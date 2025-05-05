#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>
#include "skeleton.h"
#include "timestep.h"

namespace game_engine {
	class Timestep;
	class SkeletalAnimation
	{
	public:
		enum class Path
		{
			TRANSLATION,
			ROTATION,
			SCALE
		};

		enum class InterpolationMethod
		{
			LINEAR,
			STEP,
			CUBICSPLINE
		};

		struct Channel
		{
			Path path;
			int sample_index;
			int node;
		};

		struct Sampler
		{
			std::vector<float> timestamps;
			std::vector<glm::vec4> TRS_output_values_to_be_interpolated;
			InterpolationMethod interpolation_method;
		};

		SkeletalAnimation(std::string const& name);

		void start();
		void stop();
		bool is_running() const;
		bool will_expire(const Timestep& timestep) const;
		std::string const& get_name() const { return name; }
		void set_repeat(bool repeat) { this->repeat = repeat; }
		void update(const Timestep& timestep, game_engine::Armature::Skeleton& skeleton);
		float get_duration() const { return last_keyframe_time - first_keyframe_time; }
		float get_current_time() const { return current_keyframe_time; }

		std::vector<SkeletalAnimation::Sampler> samplers;
		std::vector<SkeletalAnimation::Channel> channels;

		void set_first_keyframe_time(float first_keyframe_time) { this->first_keyframe_time = first_keyframe_time; }
		void set_last_keyframe_time(float last_keyframe_time) { this->last_keyframe_time = last_keyframe_time; }

	private:
		std::string name;
		bool repeat = false;

		float first_keyframe_time = 0.0f;
		float last_keyframe_time = 0.0f;
		float current_keyframe_time = 0.0f;
	};
}