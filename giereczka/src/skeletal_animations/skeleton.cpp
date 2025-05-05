#include <skeletal_animations/skeleton.h>

void game_engine::Armature::Skeleton::Traverse()
{
	uint32_t indent = 0;
	std::string indent_str(indent, ' ');
	auto& joint = joints[ROOT_JOINT];
	Traverse(joint, indent + 1);
}

void game_engine::Armature::Skeleton::Traverse(Joint const& joint, uint32_t indent)
{
	std::string indent_str(indent, ' ');
	size_t number_of_children = joint.children.size();
	for (size_t child_index = 0; child_index < number_of_children; ++child_index)
	{
		int joint_index = joint.children[child_index];
	}

	for (size_t child_index = 0; child_index < number_of_children; ++child_index)
	{
		int joint_index = joint.children[child_index];
		Traverse(joints[joint_index], indent + 1);
	}
}

void game_engine::Armature::Skeleton::Update()
{
	int16_t number_of_joints = static_cast<int16_t>(joints.size());
	if (!is_animated)
	{
		for (int16_t joint_index = 0; joint_index < number_of_joints; ++joint_index)
		{
			shader_data.final_joint_matrices[joint_index] = glm::mat4(1.0f);
		}
	}
	else
	{
		for (int16_t joint_index = 0; joint_index < number_of_joints; ++joint_index)
		{
			shader_data.final_joint_matrices[joint_index] = joints[joint_index].get_deformed_bind_matrix();
		}

		UpdateJoint(ROOT_JOINT);

		for (int16_t joint_index = 0; joint_index < number_of_joints; ++joint_index)
		{
			shader_data.final_joint_matrices[joint_index] = shader_data.final_joint_matrices[joint_index] * joints[joint_index].inverse_bind_matrix;
		}
	}
}

void game_engine::Armature::Skeleton::UpdateJoint(int16_t joint_index)
{
	auto& current_joint = joints[joint_index];

	int16_t parent_joint = current_joint.parent_joint;

	if (parent_joint != Armature::NO_PARENT)
	{
		shader_data.final_joint_matrices[joint_index] = shader_data.final_joint_matrices[parent_joint] * shader_data.final_joint_matrices[joint_index];
	}

	size_t number_of_children = current_joint.children.size();
	for (size_t child_index = 0; child_index < number_of_children; ++child_index)
	{
		int child_joint = current_joint.children[child_index];
		UpdateJoint(child_joint);
	}
}
