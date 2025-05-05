#pragma once

#include "imgui.h"
#include "device.h"
#include "descriptors.h"
#include "object_manager_system.h"
#include "player_controller.h"
#include "performance_counter.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "implot.h"

#include <vector>
#include <stdexcept>

namespace game_engine {
    class DebugUI {
    public:
        DebugUI(GLFWwindow* window, Device& device);
        ~DebugUI();

        void init_debug_ui(VkRenderPass render_pass, uint32_t image_count);

		bool is_free_camera() { return free_camera; }
		int get_debug_object_id() { return debug_object_id; }
		bool is_render_wireframe() { return render_wireframe; }
    	bool is_render_raytracing() { return render_raytracing; }

        void draw(
            VkCommandBuffer command_buffer,
            ObjectManagerSystem& object_manager_system,
            PlayerController& player_controller,
			PerformanceCounter& performance_counter
        );
    private:
        void init_imgui(VkRenderPass render_pass, uint32_t image_count);
        void cleanup_imgui();

        GLFWwindow* window;
        Device& device;
        VkDescriptorPool imgui_pool;
        std::unique_ptr<DescriptorPool> imgui_descriptor_pool;

		int selected_object = -1;
		int previous_selected_object = -1;

		int debug_object_id = 0;

		bool free_camera = false;

		bool render_wireframe = false;
    	bool render_raytracing = false;
    };
}