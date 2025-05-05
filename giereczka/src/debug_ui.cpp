#include "debug_ui.h"

game_engine::DebugUI::DebugUI(GLFWwindow* window, Device& device)
    : window{ window }, device{ device }
{
}

game_engine::DebugUI::~DebugUI()
{
    cleanup_imgui();
    ImGui_ImplGlfw_Shutdown();
	ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

void game_engine::DebugUI::init_debug_ui(VkRenderPass render_pass, uint32_t image_count)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	init_imgui(render_pass, image_count);
}

void game_engine::DebugUI::init_imgui(VkRenderPass render_pass, uint32_t image_count)
{
	imgui_descriptor_pool = DescriptorPool::Builder{ device }
		.add_pool_size(VK_DESCRIPTOR_TYPE_SAMPLER, 1000)
		.add_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
		.add_pool_size(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000)
		.add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000)
		.add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000)
		.add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000)
		.add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
		.add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000)
		.add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
		.add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
		.add_pool_size(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000)
        .set_pool_flags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
		.set_max_sets(1000)
		.build();

    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGui_ImplGlfw_InitForVulkan(window, true);

	ImGui_ImplVulkan_InitInfo init_info{};
	init_info.Instance = device.get_instance();
	init_info.PhysicalDevice = device.get_physical_device();
	init_info.Device = device.get_logical_device();
    init_info.Queue = device.get_graphics_queue();
    init_info.DescriptorPool = imgui_descriptor_pool->get_descriptor_pool();
	init_info.MinImageCount = image_count;
	init_info.ImageCount = image_count;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.RenderPass = render_pass;

	ImGui_ImplVulkan_Init(&init_info);

    VkCommandBuffer command_buffer = device.begin_single_time_commands();
    ImGui_ImplVulkan_CreateFontsTexture();
    device.end_single_time_commands(command_buffer);
}

void game_engine::DebugUI::cleanup_imgui()
{
    ImGui_ImplVulkan_Shutdown();
}

void game_engine::DebugUI::draw(
    VkCommandBuffer command_buffer,
    ObjectManagerSystem& object_manager_system,
    PlayerController& player_controller,
    PerformanceCounter& performance_counter
)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Debug Window");

    auto &game_objects = object_manager_system.get_game_objects();
    auto &point_lights = object_manager_system.get_point_lights();

	std::vector<std::pair<ObjectManagerSystem::id_t, GameObject>> game_objects_vector(game_objects.begin(), game_objects.end());
	std::vector<std::pair<ObjectManagerSystem::id_t, game_engine::PointLightObject>> point_lights_vector(point_lights.begin(), point_lights.end());

	int game_object_count = game_objects_vector.size();
	int point_light_count = point_lights_vector.size();

    if (ImGui::BeginTabBar("DebugTabBar"))
    {
        if (ImGui::BeginTabItem("Game objects"))
        {
            if (ImGui::BeginTable(
                "ObjectTable",
                5,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 50.f);
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 100.f);
				ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthFixed, 75.f);
				ImGui::TableSetupColumn("Y", ImGuiTableColumnFlags_WidthFixed, 75.f);
				ImGui::TableSetupColumn("Z", ImGuiTableColumnFlags_WidthFixed, 75.f);
                ImGui::TableHeadersRow();

                for (int row = 0; row < game_object_count; row++)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    if (
                        ImGui::Selectable(std::to_string(game_objects_vector[row].first).c_str(),
                        selected_object == row,
                        ImGuiSelectableFlags_SpanAllColumns
                        ))
                    {
                        selected_object = row;
                    }

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", game_objects_vector[row].second.name.c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%f", game_objects_vector[row].second.transform.translation.x);
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%f", game_objects_vector[row].second.transform.translation.y);
                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%f", game_objects_vector[row].second.transform.translation.z);
                }

				for (int row = game_object_count; row < point_light_count + game_object_count; row++)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					if (
						ImGui::Selectable(std::to_string(point_lights_vector[row - game_object_count].first).c_str(),
							selected_object == row,
							ImGuiSelectableFlags_SpanAllColumns
						))
					{
						selected_object = row;
					}
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("Point light");
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("%f", point_lights_vector[row - game_object_count].second.position.x);
					ImGui::TableSetColumnIndex(3);
					ImGui::Text("%f", point_lights_vector[row - game_object_count].second.position.y);
					ImGui::TableSetColumnIndex(4);
					ImGui::Text("%f", point_lights_vector[row - game_object_count].second.position.z);
				}
				ImGui::EndTable();
			}

            if (selected_object != -1 && selected_object < game_object_count)
            {
				glm::vec3 translation = game_objects_vector[selected_object].second.transform.translation;
				float scale = game_objects_vector[selected_object].second.transform.scale;
				glm::vec3 rotation = game_objects_vector[selected_object].second.transform.rotation;
				ImGui::DragFloat3("Translation", &translation.x, 0.01f);
				ImGui::DragFloat("Scale", &scale, 0.01f);
				ImGui::DragFloat3("Rotation", &rotation.x, 0.01f);

				object_manager_system.move_game_object(game_objects_vector[selected_object].first, translation);
				object_manager_system.scale_game_object(game_objects_vector[selected_object].first, scale);
				object_manager_system.rotate_game_object(game_objects_vector[selected_object].first, rotation);

				if (previous_selected_object != selected_object)
				{
					if (previous_selected_object != -1)
					{
						object_manager_system.set_model_color(game_objects_vector[previous_selected_object].first, glm::vec3(0.0f, 0.0f, 0.0f));
					}
					object_manager_system.set_model_color(game_objects_vector[selected_object].first, glm::vec3(0.f, 1.f, 0.f));
					debug_object_id = game_objects_vector[selected_object].first;
				}

				previous_selected_object = selected_object;
            }
			else if (selected_object != -1 && selected_object < point_light_count+game_object_count && selected_object >= game_object_count)
			{
				const int object_id = selected_object - game_object_count;

				glm::vec3 translation = point_lights_vector[object_id].second.position;
				float scale = point_lights_vector[object_id].second.light_radius;
				glm::vec3 color = point_lights_vector[object_id].second.light_color;
				float intensity = point_lights_vector[object_id].second.light_intensity;
				ImGui::DragFloat3("Translation", &translation.x, 0.01f);
				ImGui::DragFloat("Scale", &scale, 0.01f);
				ImGui::ColorEdit3("Color", &color.x);
				ImGui::DragFloat("Intensity", &intensity, 0.01f);

				object_manager_system.set_point_light_position(point_lights_vector[object_id].first, translation);
				object_manager_system.set_point_light_radius(point_lights_vector[object_id].first, scale);
				object_manager_system.set_point_light_color(point_lights_vector[object_id].first, color);
				object_manager_system.set_point_light_intensity(point_lights_vector[object_id].first, intensity);
			}
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Camera settings"))
        {
            ImGui::DragFloat("Mouse sensitivity", &player_controller.mouse_sensitivity, 0.001f);
            ImGui::DragFloat("Movement speed", &player_controller.movement_speed, 0.01f);

            ImGui::EndTabItem();
        }

		if (ImGui::BeginTabItem("Performance"))
		{
			float x_data[1000];
			float y_data[1000];

            auto data = performance_counter.get_frame_times();

			for (int i = 0; i < data.size(); i++)
			{
				x_data[i] = i;
				y_data[i] = data[i];
			}

            if (ImPlot::BeginPlot("Frametimes plot"))
            {
				ImPlot::PlotLine("Frametimes", x_data, y_data, data.size());
				ImPlot::EndPlot();
            }

			ImGui::Text("Frame time: %f", performance_counter.get_delta_time());
			ImGui::Text("FPS: %f", performance_counter.get_fps());
			ImGui::Text("Vertex count: %d", object_manager_system.get_vertex_count());
			ImGui::EndTabItem();
		}

        if (ImGui::BeginTabItem("Draw options"))
        {
            ImGui::Checkbox("Wireframe", &render_wireframe);
        	ImGui::Checkbox("Raytracing", &render_raytracing);
			ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
}
