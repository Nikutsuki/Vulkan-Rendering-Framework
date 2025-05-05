#include "skeletal_animations/skeletal_animation.h"

game_engine::SkeletalAnimation::SkeletalAnimation(std::string const& name) : name(name)
{
}

void game_engine::SkeletalAnimation::start()
{
	current_keyframe_time = first_keyframe_time;
}

void game_engine::SkeletalAnimation::stop()
{
	current_keyframe_time = last_keyframe_time + 1.0f;
}

bool game_engine::SkeletalAnimation::is_running() const
{
	return (repeat || current_keyframe_time <= last_keyframe_time);
}

bool game_engine::SkeletalAnimation::will_expire(const Timestep& timestep) const
{
	return (repeat && ((current_keyframe_time + timestep) > last_keyframe_time));
}

void game_engine::SkeletalAnimation::update(const Timestep& timestep, game_engine::Armature::Skeleton& skeleton)
{
	if (!is_running()) return;

	current_keyframe_time += timestep;

	if (repeat && (current_keyframe_time > last_keyframe_time))
	{
		current_keyframe_time = first_keyframe_time;
	}

	for (auto& channel : channels)
	{
		auto& sampler = samplers[channel.sample_index];
		int joint_index = skeleton.global_node_to_joint_index[channel.node];
		auto& joint = skeleton.joints[joint_index];

		for (size_t i = 0; i < sampler.timestamps.size() - 1; i++)
		{
			if ((current_keyframe_time >= sampler.timestamps[i]) && current_keyframe_time <= sampler.timestamps[i + 1])
			{
				switch (sampler.interpolation_method)
				{
				case InterpolationMethod::LINEAR:
				{
					float a =
						(current_keyframe_time - sampler.timestamps[i]) /
						(sampler.timestamps[i + 1] - sampler.timestamps[i]);

					switch (channel.path)
					{
					case Path::TRANSLATION:
					{
						joint.deformed_position = glm::mix(
							sampler.TRS_output_values_to_be_interpolated[i],
							sampler.TRS_output_values_to_be_interpolated[i + 1],
							a);
						break;
					}
					case Path::ROTATION:
					{
						glm::quat quaternion1;
						quaternion1.x = sampler.TRS_output_values_to_be_interpolated[i].x;
						quaternion1.y = sampler.TRS_output_values_to_be_interpolated[i].y;
						quaternion1.z = sampler.TRS_output_values_to_be_interpolated[i].z;
						quaternion1.w = sampler.TRS_output_values_to_be_interpolated[i].w;

						glm::quat quaternion2;
						quaternion2.x = sampler.TRS_output_values_to_be_interpolated[i + 1].x;
						quaternion2.y = sampler.TRS_output_values_to_be_interpolated[i + 1].y;
						quaternion2.z = sampler.TRS_output_values_to_be_interpolated[i + 1].z;
						quaternion2.w = sampler.TRS_output_values_to_be_interpolated[i + 1].w;

						joint.deformed_rotation = glm::normalize(glm::slerp(quaternion1, quaternion2, a));
						break;
					}
					case Path::SCALE:
					{
						joint.deformed_scale = glm::mix(
							glm::vec3(sampler.TRS_output_values_to_be_interpolated[i]),
							glm::vec3(sampler.TRS_output_values_to_be_interpolated[i + 1]),
							a);
						break;
					}
					default:
						throw std::runtime_error("Unknown path type");
					}
				}
				break;

				case InterpolationMethod::STEP:
				{
					switch (channel.path)
					{
					case Path::TRANSLATION:
					{
						joint.deformed_position = glm::vec3(sampler.TRS_output_values_to_be_interpolated[i]);
						break;
					}
					case Path::ROTATION:
					{
						joint.deformed_rotation.x = sampler.TRS_output_values_to_be_interpolated[i].x;
						joint.deformed_rotation.y = sampler.TRS_output_values_to_be_interpolated[i].y;
						joint.deformed_rotation.z = sampler.TRS_output_values_to_be_interpolated[i].z;
						joint.deformed_rotation.w = sampler.TRS_output_values_to_be_interpolated[i].w;
						break;
					}
					case Path::SCALE:
					{
						joint.deformed_scale = glm::vec3(sampler.TRS_output_values_to_be_interpolated[i]);
						break;
					}
					default:
						throw std::runtime_error("Unknown path type");
					}
				}
				break;

				case InterpolationMethod::CUBICSPLINE:
				{
					throw std::runtime_error("Cubic spline interpolation not implemented");
				}
				break;

				default:
					throw std::runtime_error("Unknown interpolation method");
					break;
				}
			}
		}
	}
}
