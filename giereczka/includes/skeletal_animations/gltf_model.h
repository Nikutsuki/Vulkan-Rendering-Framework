#pragma once

#include "pch.h"

#include <tiny_gltf.h>

#include "skeleton.h"
#include "skeletal_animations.h"
#include "device.h"
#include "texture.h"
#include "model.h"
#include "material.h"

namespace game_engine {
	class GltfModel {
	public:
		GltfModel(Device& device, const std::string& file_path);
		tinygltf::Model model;
        std::shared_ptr<SkeletalAnimations> animations;
        std::shared_ptr<Armature::Skeleton> skeleton;

		std::vector<std::shared_ptr<Model>> models;

        std::vector<Model::Submesh> submeshes;

        std::vector<std::shared_ptr<Texture>> textures;
        std::vector<Material> materials;
		std::vector<Material::MaterialTextures> material_textures;

		uint32_t texture_id = 0;

        Texture& get_texture(uint32_t index);
	private:
		Device& device;

		std::vector<Model::Vertex> vertices;
		std::vector<uint32_t> indices;


		void load_skeletons();
        void load_textures();
		void load_materials();

        int get_min_filter(uint32_t index);
		int get_mag_filter(uint32_t index);

		bool get_image_format(uint32_t index);

        void load_joint(int global_gltf_node_index, int parent_joint);

        void load_vertex_data(uint32_t const mesh_index);

		void calculate_tangents();
		void calculate_tangents_from_index_buffer(const std::vector<uint32_t>& indices);

        void assign_material(Model::Submesh& submesh, int const material_index);

		static constexpr int GLTF_NOT_USED = -1;

        bool skeletal_animation = false;
        uint32_t texture_offset = 0;

        template <typename T>
        int load_accessor(const tinygltf::Accessor& accessor, const T*& pointer, uint32_t* count = nullptr, int* type = nullptr)
        {
			const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
            pointer =
                reinterpret_cast<const T*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
            if (count)
            {
                *count = static_cast<uint32_t>(accessor.count);
            }
            if (type)
            {
                *type = accessor.type;
            }
            return accessor.componentType;
        }
	};
}