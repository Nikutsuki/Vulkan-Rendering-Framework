#include "skeletal_animations/gltf_model.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

tinygltf::TinyGLTF loader;

game_engine::GltfModel::GltfModel(Device& device, const std::string& file_path) : device{ device }
{
	std::string err;
	std::string warn;
	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, file_path);
	load_skeletons();
	load_textures();
	load_materials();

	for (uint32_t mesh_index = 0; mesh_index < model.meshes.size(); ++mesh_index)
	{
		load_vertex_data(mesh_index);

		auto model = std::make_shared<Model>(device, vertices, indices);
		models.push_back(model);
	}
}

game_engine::Texture& game_engine::GltfModel::get_texture(uint32_t index)
{
	assert(index < textures.size() && index >= 0 && "index out of range");
	return *textures[index];
}

void game_engine::GltfModel::load_skeletons()
{
	size_t number_of_skeletons = model.skins.size();
	if (number_of_skeletons <= 0)
	{
		return;
		//throw std::runtime_error("No skeletons found in gltf file");
	}

	if (number_of_skeletons > 1)
	{
		return;
		//throw std::runtime_error("More than one skeleton found in gltf file");
	}

	animations = std::make_shared<SkeletalAnimations>();
	skeleton = std::make_shared<Armature::Skeleton>();

	if(model.skins.size() == 1)
	{
		const tinygltf::Skin& skin = model.skins[0];

		if (skin.inverseBindMatrices != GLTF_NOT_USED)
		{
			auto& joints = skeleton->joints;

			size_t number_of_joints = skin.joints.size();

			joints.resize(number_of_joints);
			skeleton->shader_data.final_joint_matrices.resize(number_of_joints);

			skeleton->name = skin.name;

			std::cout << "Skin name: " << skin.name << std::endl;

			const glm::mat4* inverse_bind_matrices;
			{
				uint32_t count = 0;
				int type = 0;
				auto component_type = load_accessor<glm::mat4>
					(
						model.accessors[skin.inverseBindMatrices],
						inverse_bind_matrices,
						&count,
						&type
					);

				assert(type == TINYGLTF_TYPE_MAT4 && "unexpected type");
				assert(component_type == GL_FLOAT && "unexpected component type");
				assert(static_cast<size_t>(count) == number_of_joints && "accessor.count != number of joints");
			}

			for (size_t joint_index = 0; joint_index < number_of_joints; ++joint_index)
			{
				int global_gltf_node_index = skin.joints[joint_index];
				auto& joint = joints[joint_index];
				joint.inverse_bind_matrix = inverse_bind_matrices[joint_index];
				joint.name = model.nodes[global_gltf_node_index].name;

				skeleton->global_node_to_joint_index[global_gltf_node_index] = joint_index;
			}

			int root_joint = skin.joints[0];

			load_joint(root_joint, Armature::NO_PARENT);
		}

		int number_of_joints = skeleton->joints.size();
		int buffer_size = number_of_joints * sizeof(glm::mat4);


	}

	size_t number_of_animations = model.animations.size();
	for (size_t animation_index = 0; animation_index < number_of_animations; ++animation_index)
	{
		auto& gltf_animation = model.animations[animation_index];
		std::string name = gltf_animation.name;
		std::cout << "Animation name: " << name << std::endl;
		std::shared_ptr<SkeletalAnimation> animation = std::make_shared<SkeletalAnimation>(name);

		size_t number_of_samplers = gltf_animation.samplers.size();
		animation->samplers.resize(number_of_samplers);
		for (size_t sampler_index = 0; sampler_index < number_of_samplers; ++sampler_index)
		{
			tinygltf::AnimationSampler gltf_sampler = gltf_animation.samplers[sampler_index];
			auto& sampler = animation->samplers[sampler_index];

			sampler.interpolation_method = SkeletalAnimation::InterpolationMethod::LINEAR;
			if (gltf_sampler.interpolation == "STEP")
			{
				sampler.interpolation_method = SkeletalAnimation::InterpolationMethod::STEP;
			}
			else if (gltf_sampler.interpolation == "CUBICSPLINE")
			{
				sampler.interpolation_method = SkeletalAnimation::InterpolationMethod::CUBICSPLINE;
			}

			{
				uint32_t count = 0;
				const float* timestamp_buffer;
				auto component_type = load_accessor<float>
					(
						model.accessors[gltf_sampler.input],
						timestamp_buffer,
						&count
					);

				assert(component_type == GL_FLOAT && "unexpected component type");

				sampler.timestamps.resize(count);
				for (size_t index = 0; index < count; ++index)
				{
					sampler.timestamps[index] = timestamp_buffer[index];
				}
			}

			{
				uint32_t count = 0;
				int type;
				const uint32_t* buffer;
				load_accessor<uint32_t>
					(
						model.accessors[gltf_sampler.output],
						buffer,
						&count,
						&type
					);

				switch (type)
				{
				case TINYGLTF_TYPE_VEC3:
				{
					const glm::vec3* output_buffer = reinterpret_cast<const glm::vec3*>(buffer);
					sampler.TRS_output_values_to_be_interpolated.resize(count);
					for (size_t index = 0; index < count; index++)
					{
						sampler.TRS_output_values_to_be_interpolated[index] = glm::vec4(output_buffer[index], 0.0f);
					}
					break;
				}
				case TINYGLTF_TYPE_VEC4:
				{
					const glm::vec4* output_buffer = reinterpret_cast<const glm::vec4*>(buffer);
					sampler.TRS_output_values_to_be_interpolated.resize(count);
					for (size_t index = 0; index < count; index++)
					{
						sampler.TRS_output_values_to_be_interpolated[index] = glm::vec4(output_buffer[index]);
					}
					break;
				}
				default:
				{
					throw std::runtime_error("unexpected type");
				}
				}
			}
		}

		if (animation->samplers.size())
		{
			auto& sampler = animation->samplers[0];
			if (sampler.timestamps.size() >= 2)
			{
				animation->set_first_keyframe_time(sampler.timestamps[0]);
				animation->set_last_keyframe_time(sampler.timestamps.back());
			}
		}

		size_t number_of_channels = gltf_animation.channels.size();
		animation->channels.resize(number_of_channels);

		for (size_t channel_index = 0; channel_index < number_of_channels; ++channel_index)
		{
			tinygltf::AnimationChannel gltf_channel = gltf_animation.channels[channel_index];
			SkeletalAnimation::Channel& channel = animation->channels[channel_index];
			channel.sample_index = gltf_channel.sampler;
			channel.node = gltf_channel.target_node;

			if (gltf_channel.target_path == "translation")
			{
				channel.path = SkeletalAnimation::Path::TRANSLATION;
			}
			else if (gltf_channel.target_path == "rotation")
			{
				channel.path = SkeletalAnimation::Path::ROTATION;
			}
			else if (gltf_channel.target_path == "scale")
			{
				channel.path = SkeletalAnimation::Path::SCALE;
			}
			else
			{
				throw std::runtime_error("unexpected path");
			}
		}
		animations->push(animation);
	}

	skeletal_animation = (animations->size()) ? true : false;
}

void game_engine::GltfModel::load_joint(int global_gltf_node_index, int parent_joint)
{
	int current_joint = skeleton->global_node_to_joint_index[global_gltf_node_index];
	auto& joint = skeleton->joints[current_joint];

	joint.parent_joint = parent_joint;

	size_t number_of_children = model.nodes[global_gltf_node_index].children.size();

	if (number_of_children > 0)
	{
		joint.children.resize(number_of_children);
		for (size_t child_index = 0; child_index < number_of_children; ++child_index)
		{
			uint32_t global_gltf_node_index_for_child = model.nodes[global_gltf_node_index].children[child_index];
			joint.children[child_index] = skeleton->global_node_to_joint_index[global_gltf_node_index_for_child];
			load_joint(global_gltf_node_index_for_child, current_joint);
		}
	}
}

void game_engine::GltfModel::load_textures()
{
	texture_offset = textures.size();
	size_t num_textures = model.images.size();
	textures.resize(texture_offset + num_textures);

	for (uint32_t image_index = 0; image_index < num_textures; ++image_index)
	{
		std::string image_file_path = model.images[image_index].uri;
		tinygltf::Image& gltf_image = model.images[image_index];

		unsigned char* buffer;
		uint64_t buffer_size;

		if (gltf_image.component == 3)
		{
			buffer_size = gltf_image.width * gltf_image.height * 4;
			std::vector<unsigned char> image_data(buffer_size, 0x0);

			buffer = (unsigned char*)image_data.data();
			unsigned char* rgba = buffer;
			unsigned char* rgb = &gltf_image.image[0];
			for (int j = 0; j < gltf_image.width * gltf_image.height; ++j)
			{
				memcpy(rgba, rgb, sizeof(unsigned char) * 3);
				rgba += 4;
				rgb += 3;
			}
		}
		else
		{
			buffer = &gltf_image.image[0];
			buffer_size = gltf_image.image.size();
		}

		auto texture = std::make_shared<Texture>(device);
		int min_filter = get_min_filter(image_index);
		int mag_filter = get_mag_filter(image_index);
		bool image_format = get_image_format(image_index);

		texture->init(gltf_image.width, gltf_image.height, image_format, buffer, min_filter, mag_filter);
		texture->set_file_name(image_file_path);

		textures[image_index] = texture;
	}
}

int game_engine::GltfModel::get_min_filter(uint32_t index)
{
	int sampler = model.textures[index].sampler;
	int filter = model.samplers[sampler].minFilter;
	std::string& name = model.images[index].name;

	switch (filter)
	{
	case TINYGLTF_TEXTURE_FILTER_NEAREST:
	case TINYGLTF_TEXTURE_FILTER_LINEAR:
	case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
	case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
	case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
	case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
		break;
	case GLTF_NOT_USED:
		filter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
		break;
	default:
	{
		filter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
		std::cout << "Unsupported min filter: " << filter << " for texture: " << name << std::endl;
		break;
	}
	}

	return filter;
}

int game_engine::GltfModel::get_mag_filter(uint32_t index)
{
	int sampler = model.textures[index].sampler;
	int filter = model.samplers[sampler].minFilter;
	std::string& name = model.images[index].name;

	switch (filter)
	{
	case TINYGLTF_TEXTURE_FILTER_NEAREST:
	case TINYGLTF_TEXTURE_FILTER_LINEAR:
	case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
	case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
	case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
	case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
		break;
	case GLTF_NOT_USED:
		filter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
		break;
	default:
	{
		filter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
		std::cout << "Unsupported min filter: " << filter << " for texture: " << name << std::endl;
		break;
	}
	}

	return filter;
}

bool game_engine::GltfModel::get_image_format(uint32_t index)
{
	for (uint32_t i = 0; i < model.materials.size(); i++)
	{
		tinygltf::Material& gltf_material = model.materials[i];

		if (static_cast<uint32_t>(gltf_material.pbrMetallicRoughness.baseColorTexture.index) == index)
		{
			return Texture::USE_SRGB;
		}
		else if (static_cast<uint32_t>(gltf_material.emissiveTexture.index) == index)
		{
			return Texture::USE_SRGB;
		}
		else if (gltf_material.values.find("baseColorTexture") != gltf_material.values.end())
		{
			int diffuse_texture_index = gltf_material.values["baseColorTexture"].TextureIndex();
			tinygltf::Texture& diffuse_texture = model.textures[diffuse_texture_index];
			if (static_cast<uint32_t>(diffuse_texture.source) == index)
			{
				return Texture::USE_SRGB;
			}
		}
	}

	return Texture::USE_UNORM;
}

void game_engine::GltfModel::load_vertex_data(uint32_t const mesh_index)
{
	vertices.clear();
	indices.clear();
	submeshes.clear();

	uint32_t num_primitives = model.meshes[mesh_index].primitives.size();
	submeshes.resize(num_primitives);

	uint32_t primitive_index = 0;
	for (const auto& gltf_primitive : model.meshes[mesh_index].primitives)
	{
		Model::Submesh& submesh = submeshes[primitive_index];
		++primitive_index;

		submesh.first_vertex = vertices.size();
		submesh.first_index = indices.size();

		uint32_t vertex_count = 0;
		uint32_t index_count = 0;

		glm::vec4 diffuse_color = glm::vec4(1.0f);
		if (gltf_primitive.material != GLTF_NOT_USED)
		{
			size_t material_index = gltf_primitive.material;
			diffuse_color = materials[material_index].pbr_material_properties.diffuse_color;
		}

		{
			const float* position_buffer = nullptr;
			const float* color_buffer = nullptr;
			const float* normals_buffer = nullptr;
			const float* tangents_buffer = nullptr;
			const float* uvs_buffer = nullptr;
			const uint32_t* joints_buffer = nullptr;
			const float* weights_buffer = nullptr;

			int joints_buffer_data_type = 0;

			if (gltf_primitive.attributes.find("POSITION") != gltf_primitive.attributes.end())
			{
				auto component_type = load_accessor<float>
					(
						model.accessors[gltf_primitive.attributes.find("POSITION")->second],
						position_buffer,
						&vertex_count
					);
				assert(component_type == GL_FLOAT && "unexpected component type");
			}

			if (gltf_primitive.attributes.find("COLOR_0") != gltf_primitive.attributes.end())
			{
				auto component_type = load_accessor<float>
					(
						model.accessors[gltf_primitive.attributes.find("COLOR_0")->second],
						color_buffer
					);
				assert(component_type == GL_FLOAT && "unexpected component type");
			}

			if (gltf_primitive.attributes.find("NORMAL") != gltf_primitive.attributes.end())
			{
				auto component_type = load_accessor<float>
					(
						model.accessors[gltf_primitive.attributes.find("NORMAL")->second],
						normals_buffer
					);
				assert(component_type == GL_FLOAT && "unexpected component type");
			}

			if (gltf_primitive.attributes.find("TANGENT") != gltf_primitive.attributes.end())
			{
				auto component_type = load_accessor<float>
					(
						model.accessors[gltf_primitive.attributes.find("TANGENT")->second],
						tangents_buffer
					);
				assert(component_type == GL_FLOAT && "unexpected component type");
			}

			if (gltf_primitive.attributes.find("TEXCOORD_0") != gltf_primitive.attributes.end())
			{
				auto component_type = load_accessor<float>
					(
						model.accessors[gltf_primitive.attributes.find("TEXCOORD_0")->second],
						uvs_buffer
					);
				assert(component_type == GL_FLOAT && "unexpected component type");
			}

			if (gltf_primitive.attributes.find("JOINTS_0") != gltf_primitive.attributes.end())
			{
				joints_buffer_data_type = load_accessor<uint32_t>
					(
						model.accessors[gltf_primitive.attributes.find("JOINTS_0")->second],
						joints_buffer
					);
				assert(joints_buffer_data_type == GL_UNSIGNED_BYTE || joints_buffer_data_type == GL_BYTE && "unexpected component type");
			}

			if (gltf_primitive.attributes.find("WEIGHTS_0") != gltf_primitive.attributes.end())
			{
				auto component_type = load_accessor<float>
					(
						model.accessors[gltf_primitive.attributes.find("WEIGHTS_0")->second],
						weights_buffer
					);
				assert(component_type == GL_FLOAT && "unexpected component type");
			}

			uint32_t num_vertices_before = vertices.size();
			vertices.resize(num_vertices_before + vertex_count);
			uint32_t vertex_index = num_vertices_before;

			for (size_t vertex_iterator = 0; vertex_iterator < vertex_count; ++vertex_iterator)
			{
				Model::Vertex vertex{};

				auto position = glm::make_vec3(&position_buffer[vertex_iterator * 3]);
				vertex.position = glm::vec3(position.x, position.y, position.z);

				vertex.normal = glm::normalize(
					glm::vec3(glm::make_vec3(&normals_buffer[vertex_iterator * 3]))
				);

				auto vertex_color = color_buffer ? glm::make_vec3(&color_buffer[vertex_iterator * 3]) : glm::vec3(1.0f);
				vertex.color = glm::vec4(vertex_color.x, vertex_color.y, vertex_color.z, 1.0f) * diffuse_color;

				vertex.uv = glm::make_vec2(&uvs_buffer[vertex_iterator * 2]);

				auto tangent = tangents_buffer ? glm::make_vec4(&tangents_buffer[vertex_iterator * 4]) : glm::vec4(0.0f);
				vertex.tangent = glm::vec3(tangent.x, tangent.y, tangent.z) * tangent.w;

				if (joints_buffer && weights_buffer)
				{
					switch (joints_buffer_data_type)
					{
					case GL_BYTE:
					case GL_UNSIGNED_BYTE:
						vertex.joint_ids = glm::ivec4(
							glm::make_vec4(&(reinterpret_cast<const int8_t*>(joints_buffer)[vertex_iterator * 4])
							));
						break;
					case GL_SHORT:
					case GL_UNSIGNED_SHORT:
						vertex.joint_ids = glm::ivec4(
							glm::make_vec4(&(reinterpret_cast<const int16_t*>(joints_buffer)[vertex_iterator * 4])
							));
						break;
					case GL_INT:
					case GL_UNSIGNED_INT:
						vertex.joint_ids = glm::ivec4(
							glm::make_vec4(&(reinterpret_cast<const int32_t*>(joints_buffer)[vertex_iterator * 4])
							));
						break;
					default:
						throw std::runtime_error("unexpected joint data type");
						break;
					}

					vertex.joint_weights = glm::make_vec4(&weights_buffer[vertex_iterator * 4]);
				}
				vertices[vertex_index] = vertex;
				++vertex_index;
			}

			if (!tangents_buffer)
			{
				calculate_tangents();
			}
		}

		{
			const uint32_t* index_buffer = nullptr;
			uint32_t count = 0;
			auto component_type = load_accessor<uint32_t>
				(
					model.accessors[gltf_primitive.indices],
					index_buffer,
					&count
				);

			index_count = count;

			switch (component_type)
			{
			case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
			{
				const uint32_t* buffer = index_buffer;
				for (size_t index = 0; index < count; ++index)
				{
					indices.push_back(buffer[index]);
				}
				break;
			}
			case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
			{
				const uint16_t* buffer = reinterpret_cast<const uint16_t*>(index_buffer);
				for (size_t index = 0; index < count; ++index)
				{
					indices.push_back(buffer[index]);
				}
				break;
			}
			case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
			{
				const uint8_t* buffer = reinterpret_cast<const uint8_t*>(index_buffer);
				for (size_t index = 0; index < count; ++index)
				{
					indices.push_back(buffer[index]);
				}
				break;
			}
			default:
				throw std::runtime_error("unexpected component type");
				break;
			}
		}

		submesh.index_count = index_count;
		submesh.vertex_count = vertex_count;
	}
}

void game_engine::GltfModel::calculate_tangents()
{
	if (indices.size() != 0)
	{
		calculate_tangents_from_index_buffer(indices);
	}
	else
	{
		uint32_t vertex_count = vertices.size();
		if (vertex_count)
		{
			std::vector<uint32_t> indices;
			indices.resize(vertex_count);
			for (uint32_t i = 0; i < vertex_count; i++)
			{
				indices[i] = i;
			}
			calculate_tangents_from_index_buffer(indices);
		}
	}
}

void game_engine::GltfModel::calculate_tangents_from_index_buffer(const std::vector<uint32_t>& indices)
{
	uint32_t cnt = 0;
	uint32_t vertex_index1 = 0;
	uint32_t vertex_index2 = 0;
	uint32_t vertex_index3 = 0;

	glm::vec position1 = glm::vec3(0.0f);
	glm::vec position2 = glm::vec3(0.0f);
	glm::vec position3 = glm::vec3(0.0f);

	glm::vec2 uv1 = glm::vec2(0.0f);
	glm::vec2 uv2 = glm::vec2(0.0f);
	glm::vec2 uv3 = glm::vec2(0.0f);

	for (uint32_t index : indices)
	{
		auto& vertex = vertices[index];

		switch (cnt)
		{
		case 0:
			position1 = vertex.position;
			uv1 = vertex.uv;
			vertex_index1 = index;
			break;
		case 1:
			position2 = vertex.position;
			uv2 = vertex.uv;
			vertex_index2 = index;
			break;
		case 2:
			position3 = vertex.position;
			uv3 = vertex.uv;
			vertex_index3 = index;
			
			glm::vec3 edge1 = position2 - position1;
			glm::vec3 edge2 = position3 - position1;

			glm::vec2 deltaUV1 = uv2 - uv1;
			glm::vec2 deltaUV2 = uv3 - uv1;

			float dU1 = deltaUV1.x;
			float dU2 = deltaUV2.x;
			float dV1 = deltaUV1.y;
			float dV2 = deltaUV2.y;
			float E1x = edge1.x;
			float E1y = edge1.y;
			float E1z = edge1.z;
			float E2x = edge2.x;
			float E2y = edge2.y;
			float E2z = edge2.z;

			float factor;
			if ((dU1 * dV2 - dU2 * dV1) > std::numeric_limits<float>::epsilon())
			{
				factor = 1.0f / (dU1 * dV2 - dU2 * dV1);
			}
			else
			{
				factor = 100000.0f;
			}

			glm::vec3 tangent;

			tangent.x = factor * (dV2 * E1x - dV1 * E2x);
			tangent.y = factor * (dV2 * E1y - dV1 * E2y);
			tangent.z = factor * (dV2 * E1z - dV1 * E2z);
			if (tangent.x == 0.0f && tangent.y == 0.0f && tangent.z == 0.0f)
			{
				tangent = glm::vec3(1.0f, 0.0f, 0.0f);
			}

			vertices[vertex_index1].tangent += tangent;
			vertices[vertex_index2].tangent += tangent;
			vertices[vertex_index3].tangent += tangent;

			break;
		}

		cnt = (cnt + 1) % 3;
	}
}

void game_engine::GltfModel::load_materials()
{
	size_t num_materials = model.materials.size();
	materials.resize(num_materials);
	material_textures.resize(num_materials);

	uint32_t material_index = 0;
	for (Material& material : materials)
	{
		tinygltf::Material& gltf_material = model.materials[material_index];
		Material::PbrMaterialProperties& pbr_material_properties = material.pbr_material_properties;
		Material::MaterialTextures& local_material_textures = material_textures[material_index];

		if (gltf_material.values.find("baseColorFactor") != gltf_material.values.end())
		{
			pbr_material_properties.diffuse_color = glm::make_vec4(gltf_material.values["baseColorFactor"].ColorFactor().data());
		}

		if (gltf_material.pbrMetallicRoughness.baseColorTexture.index != GLTF_NOT_USED)
		{
			int diffuse_texture_index = gltf_material.pbrMetallicRoughness.baseColorTexture.index;
			tinygltf::Texture& diffuse_texture = model.textures[diffuse_texture_index];
			local_material_textures[Material::DIFFUSE_MAP_INDEX] = textures[diffuse_texture.source];
			pbr_material_properties.material_features |= Material::HAS_DIFFUSE_MAP;
		}
		else if (gltf_material.values.find("baseColorTexture") != gltf_material.values.end())
		{
			int diffuse_texture_index = gltf_material.values["baseColorTexture"].TextureIndex();
			tinygltf::Texture& diffuse_texture = model.textures[diffuse_texture_index];
			local_material_textures[Material::DIFFUSE_MAP_INDEX] = textures[diffuse_texture.source];
			pbr_material_properties.material_features |= Material::HAS_DIFFUSE_MAP;
		}

		if (gltf_material.normalTexture.index != GLTF_NOT_USED)
		{
			int normal_texture_index = gltf_material.normalTexture.index;
			tinygltf::Texture& normal_texture = model.textures[normal_texture_index];
			local_material_textures[Material::NORMAL_MAP_INDEX] = textures[normal_texture.source];
			pbr_material_properties.normal_map_intensity = gltf_material.normalTexture.scale;
			pbr_material_properties.material_features |= Material::HAS_NORMAL_MAP;
		}

		{
			pbr_material_properties.roughness = gltf_material.pbrMetallicRoughness.roughnessFactor;
			pbr_material_properties.metallic = gltf_material.pbrMetallicRoughness.metallicFactor;
		}

		if (gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index != GLTF_NOT_USED)
		{
			uint32_t metallic_rougness_texture_index = gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index;
			tinygltf::Texture& metallic_roughness_texture = model.textures[metallic_rougness_texture_index];
			local_material_textures[Material::ROUGHNESS_METALLIC_MAP_INDEX] = textures[metallic_roughness_texture.source];
			pbr_material_properties.material_features |= Material::HAS_ROUGHNESS_METALLIC_MAP;
		}

		if (gltf_material.emissiveFactor.size() == 3)
		{
			pbr_material_properties.emissive_color = glm::make_vec3(gltf_material.emissiveFactor.data());

			pbr_material_properties.emissive_intensity = 1.0f;
			auto it = gltf_material.extensions.find("KHR_materials_emissive_strength");
			if (it != gltf_material.extensions.end())
			{
				auto extension = it->second;
				if (extension.IsObject())
				{
					auto emissive_strength = extension.Get("emissiveStrength");
					if (emissive_strength.IsReal())
					{
						pbr_material_properties.emissive_intensity = emissive_strength.GetNumberAsDouble();
					}
				}
			}
		}

		if (gltf_material.emissiveTexture.index != GLTF_NOT_USED)
		{
			int emissive_texture_index = gltf_material.emissiveTexture.index;
			tinygltf::Texture& emissive_texture = model.textures[emissive_texture_index];
			local_material_textures[Material::EMISSIVE_MAP_INDEX] = textures[emissive_texture.source];
			pbr_material_properties.material_features |= Material::HAS_EMISSIVE_MAP;
		}

		++material_index;
	}
}

void game_engine::GltfModel::assign_material(Model::Submesh& submesh, int const material_index)
{
	{
		if (!(static_cast<size_t>(material_index) < materials.size()))
		{
			throw std::runtime_error("material index out of range");
		}

		auto material = std::make_shared<Material>();
		submesh.material = material;

		if (material_index != GLTF_NOT_USED)
		{
			*material = materials[material_index];
			material->material_textures = material_textures[material_index];
		}
	}
}