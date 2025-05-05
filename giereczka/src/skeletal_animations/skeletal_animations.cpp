#include "skeletal_animations/skeletal_animations.h"

game_engine::SkeletalAnimations::Iterator::Iterator(SkeletalAnimation* pointer)
{
	this->pointer = pointer;
}

game_engine::SkeletalAnimations::Iterator::Iterator(std::shared_ptr<SkeletalAnimation>* pointer)
{
	this->pointer = pointer->get();
}

game_engine::SkeletalAnimations::Iterator& game_engine::SkeletalAnimations::Iterator::operator++()
{
	++pointer;
	return *this;
}

bool game_engine::SkeletalAnimations::Iterator::operator!=(const Iterator& other)
{
	return pointer != other.pointer;
}

game_engine::SkeletalAnimation& game_engine::SkeletalAnimations::Iterator::operator*()
{
	return (*this->pointer);
}

game_engine::SkeletalAnimations::Iterator game_engine::SkeletalAnimations::begin()
{
	return Iterator(&(*animation_pointers.begin()));
}

game_engine::SkeletalAnimations::Iterator game_engine::SkeletalAnimations::end()
{
	return Iterator(&(*animation_pointers.end()));
}

game_engine::SkeletalAnimation& game_engine::SkeletalAnimations::operator[](int index)
{
	return *animation_pointers[index];
}

game_engine::SkeletalAnimation& game_engine::SkeletalAnimations::operator[](const std::string& name)
{
	return *animations[name];
}

game_engine::SkeletalAnimations::SkeletalAnimations() : current_animation(nullptr), current_animation_index(1)
{
}

void game_engine::SkeletalAnimations::push(std::shared_ptr<SkeletalAnimation> const& animation)
{
	if (animation)
	{
		animations[animation->get_name()] = animation;
		animation_pointers.push_back(animation);
		animation_name_to_index[animation->get_name()] = animation_pointers.size() - 1;
	}
	else
	{
		throw std::runtime_error("SkeletalAnimations::push: animation is nullptr");
	}
}

void game_engine::SkeletalAnimations::start(const std::string& name)
{
	SkeletalAnimation* local_current_animation = animations[name].get();
	if (local_current_animation)
	{
		current_animation = local_current_animation;
		current_animation->start();
	}
}

void game_engine::SkeletalAnimations::start(size_t index)
{
	if (!(index < animation_pointers.size()))
	{
		throw std::runtime_error("SkeletalAnimations::start: index out of range");
	}

	SkeletalAnimation* local_current_animation = animation_pointers[index].get();
	if (local_current_animation)
	{
		current_animation = local_current_animation;
		current_animation_index = index;
		current_animation->start();
	}
}

void game_engine::SkeletalAnimations::stop()
{
	if (current_animation)
	{
		current_animation->stop();
	}
}

void game_engine::SkeletalAnimations::set_repeat(bool repeat)
{
	if (current_animation)
	{
		current_animation->set_repeat(repeat);
	}
}

void game_engine::SkeletalAnimations::set_repeat_all(bool repeat)
{
	for (auto& animation : animation_pointers)
	{
		animation->set_repeat(repeat);
	}
}

bool game_engine::SkeletalAnimations::is_running() const
{
	return current_animation && current_animation->is_running();
}

bool game_engine::SkeletalAnimations::will_expire(const Timestep& timestep) const
{
	return current_animation && current_animation->will_expire(timestep);
}

float game_engine::SkeletalAnimations::get_duration(std::string const& name)
{
	return animations[name]->get_duration();
}

float game_engine::SkeletalAnimations::get_current_time() const
{
	return current_animation ? current_animation->get_current_time() : 0.0f;
}

std::string game_engine::SkeletalAnimations::get_name()
{
	return current_animation ? current_animation->get_name() : "";
}

void game_engine::SkeletalAnimations::update(const Timestep& timestep, game_engine::Armature::Skeleton& skeleton, uint32_t local_current_animation_index)
{
	//if (local_current_animation_index != current_animation_index)
	//{
	//	current_animation_index = local_current_animation_index;

	//	if (current_animation)
	//	{
	//		current_animation->update(timestep, skeleton);
	//	}
	//}

	if (current_animation)
	{
		current_animation->update(timestep, skeleton);
	}
}

int game_engine::SkeletalAnimations::get_index(const std::string& name)
{
	bool found = false;
	for (auto& element : animation_pointers)
	{
		if (element->get_name() == name)
		{
			found = true;
			break;
		}
	}

	if (!found) return -1;
	else animation_name_to_index[name];
}