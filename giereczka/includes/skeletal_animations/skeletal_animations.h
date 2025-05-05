#pragma once

#include "skeletal_animation.h"

#include <map>

namespace game_engine {
	class SkeletalAnimations
	{
	public:
		struct Iterator
		{
		public:
			Iterator(SkeletalAnimation* pointer);
			Iterator(std::shared_ptr<SkeletalAnimation>* pointer);
			Iterator& operator++();
			bool operator!=(const Iterator& other);
			SkeletalAnimation& operator*();
		private:
			SkeletalAnimation* pointer;
		};

		Iterator begin();
		Iterator end();
		SkeletalAnimation& operator[](int index);
		SkeletalAnimation& operator[](const std::string& name);

		SkeletalAnimations();

		size_t size() const { return animations.size(); };
		void push(std::shared_ptr<SkeletalAnimation> const& animation);

		void start(const std::string& name);
		void start(size_t index);
		void start() { start(0); };
		void stop();
		void set_repeat(bool repeat);
		void set_repeat_all(bool repeat);
		bool is_running() const;
		bool will_expire(const Timestep& timestep) const;
		float get_duration(std::string const& name);
		float get_current_time() const;
		std::string get_name();
		void update(const Timestep& timestep, game_engine::Armature::Skeleton& skeleton, uint32_t local_current_animation_index);
		int get_index(const std::string& name);
	private:
		std::map<std::string, std::shared_ptr<SkeletalAnimation>> animations;
		std::vector<std::shared_ptr<SkeletalAnimation>> animation_pointers;
		SkeletalAnimation* current_animation;
		uint32_t current_animation_index;
		std::map<std::string, uint32_t> animation_name_to_index;
	};
}