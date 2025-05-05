#pragma once

#include <memory>
#include <vector>
#include <chrono>

#include "window.h"
#include "device.h"
#include "renderer.h"
#include "systems/render_system.h"
#include "systems/point_light_system.h"
#include "camera.h"
#include "player_controller.h"
#include "topaz_input_system.h"
#include "buffer.h"
#include "descriptors.h"
#include "global_ubo.h"
#include "systems/object_manager_system.h"
#include "debug_ui.h"
#include "performance_counter.h"
#include "skeletal_animations/gltf_model.h"
#include "buffers/skeletal_animation_uniform.h"
#include "texture.h"
#include "descriptors/material_descriptor.h"
#include "systems/texture_manager_system.h"
#include "systems/raytracing_render_system.h"

namespace game_engine {
	class Engine {
	public:
		static constexpr int WIDTH = 1366;
		static constexpr int HEIGHT = 768;

		Engine();

		~Engine();

		Engine(const Engine &) = delete;

		Engine &operator=(const Engine &) = delete;

		void run();

	private:
		void load_game_objects(TextureManagerSystem &texture_manager_system);

		Window window{WIDTH, HEIGHT, "Vulkan"};
		Device device{window};
		Renderer renderer{window, device};
		ObjectManagerSystem object_manager_system;
		DebugUI debug_ui{window.get_window(), device};

		std::unique_ptr<DescriptorPool> global_descriptor_pool;
		std::unique_ptr<DescriptorSetLayout> global_descriptor_set_layout;
		std::unique_ptr<DescriptorSetLayout> joint_descriptor_set_layout;
		std::unique_ptr<DescriptorSetLayout> materials_descriptor_set_layout;
		std::unique_ptr<DescriptorSetLayout> mesh_data_set_layout;
	};
}
