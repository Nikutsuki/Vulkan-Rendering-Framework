#pragma once

#include "device.h"
#include "buffer.h"
#include "utils.h"
#include "material.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <vector>

namespace game_engine {
	class Model {
	public:
		struct Vertex {
			glm::vec3 position{}; // layout(location = 0)
			glm::vec3 color{}; // layout(location = 1)
			glm::vec3 normal{}; // layout(location = 2)
			glm::vec2 uv{}; // layout(location = 3)
			glm::vec3 tangent{}; // layout(location = 4)
			glm::ivec4 joint_ids{};	// layout(location = 5)
			glm::vec4 joint_weights{}; // layout(location = 6)

			static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
			static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();

			bool operator==(const Vertex& other) const {
				return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
			};
		};

		struct Submesh {
			uint32_t first_index;
			uint32_t first_vertex;
			uint32_t index_count;
			uint32_t vertex_count;
			uint32_t instance_count;
			std::shared_ptr<Material> material;
		};

		Model(Device& device, std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		~Model();

		Model(const Model&) = delete;
		void operator=(const Model&) = delete;

		std::vector<Vertex>& get_vertices();
		std::vector<uint32_t>& get_indices();

		uint32_t get_vertex_count() const { return vertex_count; }

		void bind(VkCommandBuffer command_buffer);
		void draw(VkCommandBuffer command_buffer);
	private:
		void create_vertex_buffers(const std::vector<Vertex> &vertices);
		void create_index_buffers(const std::vector<uint32_t>& indices);

		Device& device;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		std::unique_ptr<Buffer> vertex_buffer;
		uint32_t vertex_count;

		bool has_index_buffer = false;
		std::unique_ptr<Buffer> index_buffer;
		uint32_t index_count;

		const std::string file_path;
	};
}