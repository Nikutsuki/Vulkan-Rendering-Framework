#include "model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace std {
	template<>
	struct hash<game_engine::Model::Vertex> {
		size_t operator()(game_engine::Model::Vertex const& vertex) const {
			size_t seed = 0;
			game_engine::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

std::vector<VkVertexInputBindingDescription> game_engine::Model::Vertex::get_binding_descriptions()
{
	std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
	binding_descriptions[0].binding = 0;
	binding_descriptions[0].stride = sizeof(Vertex);
	binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return binding_descriptions;
}

std::vector<VkVertexInputAttributeDescription> game_engine::Model::Vertex::get_attribute_descriptions()
{
	std::vector<VkVertexInputAttributeDescription> attribute_descriptions(7);

	attribute_descriptions[0].binding = 0;
	attribute_descriptions[0].location = 0;
	attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_descriptions[0].offset = offsetof(Vertex, position);

	attribute_descriptions[1].binding = 0;
	attribute_descriptions[1].location = 1;
	attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_descriptions[1].offset = offsetof(Vertex, color);

	attribute_descriptions[2].binding = 0;
	attribute_descriptions[2].location = 2;
	attribute_descriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_descriptions[2].offset = offsetof(Vertex, normal);

	attribute_descriptions[3].binding = 0;
	attribute_descriptions[3].location = 3;
	attribute_descriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
	attribute_descriptions[3].offset = offsetof(Vertex, uv);

	attribute_descriptions[4].binding = 0;
	attribute_descriptions[4].location = 4;
	attribute_descriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribute_descriptions[4].offset = offsetof(Vertex, tangent);

	attribute_descriptions[5].binding = 0;
	attribute_descriptions[5].location = 5;
	attribute_descriptions[5].format = VK_FORMAT_R32G32B32A32_SINT;
	attribute_descriptions[5].offset = offsetof(Vertex, joint_ids);

	attribute_descriptions[6].binding = 0;
	attribute_descriptions[6].location = 6;
	attribute_descriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attribute_descriptions[6].offset = offsetof(Vertex, joint_weights);

	return attribute_descriptions;
}

game_engine::Model::Model(Device& device, std::vector<Vertex> vertices, std::vector<uint32_t> indices) : device(device), file_path(file_path)
{
	create_vertex_buffers(vertices);
	create_index_buffers(indices);
}

game_engine::Model::~Model()
{
}

std::vector<game_engine::Model::Vertex>& game_engine::Model::get_vertices()
{
	return vertices;
}

std::vector<uint32_t>& game_engine::Model::get_indices()
{
	return indices;
}

void game_engine::Model::bind(VkCommandBuffer command_buffer)
{
	VkBuffer buffers[] = { vertex_buffer->get_buffer()};
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

	if (has_index_buffer)
	{
		vkCmdBindIndexBuffer(command_buffer, index_buffer->get_buffer(), 0, VK_INDEX_TYPE_UINT32);
	}
}

void game_engine::Model::draw(VkCommandBuffer command_buffer)
{
	if (has_index_buffer)
	{
		vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);
	}
	else
	{
		vkCmdDraw(command_buffer, vertex_count, 1, 0, 0);
	}
}

void game_engine::Model::create_vertex_buffers(const std::vector<Vertex>& vertices)
{
	this->vertices = vertices;
	vertex_count = static_cast<uint32_t>(vertices.size());

	assert(vertex_count >= 3 && "Vertex count must be at least 3");

	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertex_count;

	uint32_t vertex_size = sizeof(vertices[0]);

	Buffer staging_buffer{
		device,
		vertex_size,
		vertex_count,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	staging_buffer.map();
	staging_buffer.write_to_buffer((void*)vertices.data());

	vertex_buffer = std::make_unique<Buffer>(
		device,
		vertex_size,
		vertex_count,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	device.copy_buffer(staging_buffer.get_buffer(), vertex_buffer->get_buffer(), bufferSize);
}

void game_engine::Model::create_index_buffers(const std::vector<uint32_t>& indices)
{
	this->indices = indices;
	index_count = static_cast<uint32_t>(indices.size());
	has_index_buffer = index_count > 0;

	if (!has_index_buffer)
	{
		return;
	}

	VkDeviceSize bufferSize = sizeof(indices[0]) * index_count;

	uint32_t index_size = sizeof(indices[0]);

	Buffer staging_buffer{
		device,
		index_size,
		index_count,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	staging_buffer.map();
	staging_buffer.write_to_buffer((void*)indices.data());

	index_buffer = std::make_unique<Buffer>(
		device,
		index_size,
		index_count,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	device.copy_buffer(staging_buffer.get_buffer(), index_buffer->get_buffer(), bufferSize);
}