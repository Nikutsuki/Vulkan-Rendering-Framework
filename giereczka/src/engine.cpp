#include "engine.h"

game_engine::Engine::Engine()
{
}

game_engine::Engine::~Engine()
{
}

void game_engine::Engine::run()
{
	global_descriptor_pool = DescriptorPool::Builder{device}
	                         .set_max_sets(SwapChain::MAX_FRAMES_IN_FLIGHT * 4)
	                         .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT * 2)
	                         .add_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	                                        SwapChain::MAX_FRAMES_IN_FLIGHT * 1024)
	                         .add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT * 4)
	                         .set_pool_flags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT)
	                         .build();

	global_descriptor_set_layout = DescriptorSetLayout::Builder{device}
	                               .add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	                                            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT |
	                                            VK_SHADER_STAGE_FRAGMENT_BIT)
	                               .build();

	joint_descriptor_set_layout = DescriptorSetLayout::Builder{device}
	                              .add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	                                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT |
	                                           VK_SHADER_STAGE_FRAGMENT_BIT)
	                              .build();

	auto materials_descriptor = std::make_unique<
		MaterialDescriptor>(MaterialDescriptor(device, global_descriptor_pool));
	std::vector<std::unique_ptr<Buffer>> uniform_buffers{SwapChain::MAX_FRAMES_IN_FLIGHT};
	std::vector<std::unique_ptr<Buffer>> joint_buffers{SwapChain::MAX_FRAMES_IN_FLIGHT};
	for (int i = 0; i < uniform_buffers.size(); i++)
	{
		uniform_buffers[i] = std::make_unique<Buffer>(
			device,
			sizeof(game_engine::GlobalUbo),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			device.properties.limits.minUniformBufferOffsetAlignment
		);
		uniform_buffers[i]->map();
	}

	for (int i = 0; i < joint_buffers.size(); i++)
	{
		joint_buffers[i] = std::make_unique<Buffer>(
			device,
			sizeof(JointUbo),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			device.properties.limits.minUniformBufferOffsetAlignment
		);
		joint_buffers[i]->map();
	}

	std::vector<VkDescriptorSet> global_descriptor_sets(SwapChain::MAX_FRAMES_IN_FLIGHT);
	std::vector<VkDescriptorSet> joint_descriptor_sets(SwapChain::MAX_FRAMES_IN_FLIGHT);

	std::vector<VkDescriptorSet> mesh_descriptor_sets(SwapChain::MAX_FRAMES_IN_FLIGHT);

	// Create descriptor sets for global and joint buffers
	for (int i = 0; i < global_descriptor_sets.size(); i++)
	{
		auto buffer_info = uniform_buffers[i]->descriptor_info();
		bool result = DescriptorWriter(*global_descriptor_set_layout, *global_descriptor_pool)
		              .write_buffer(0, &buffer_info)
		              .build(global_descriptor_sets[i]);
		assert(result && "Failed to build global descriptor set");
	}

	for (int i = 0; i < joint_descriptor_sets.size(); i++)
	{
		auto buffer_info = joint_buffers[i]->descriptor_info();
		bool result = DescriptorWriter(*joint_descriptor_set_layout, *global_descriptor_pool)
		              .write_buffer(0, &buffer_info)
		              .build(joint_descriptor_sets[i]);
		assert(result && "Failed to build joint descriptor set");
	}

	RenderSystem render_system{
		device,
		renderer.get_swap_chain_render_pass(),
		global_descriptor_set_layout->get_descriptor_set_layout(),
		joint_descriptor_set_layout->get_descriptor_set_layout(),
		materials_descriptor->get_descriptor_set_layout()
	};

	PointLightSystem point_light_system{
		device, renderer.get_swap_chain_render_pass(), global_descriptor_set_layout->get_descriptor_set_layout()
	};

	PlayerController player_controller;
	TopazInputSystem input_system(window.get_window(), player_controller);

	TextureManagerSystem texture_manager_system(materials_descriptor);

	load_game_objects(texture_manager_system);

	PerformanceCounter performance_counter;

	debug_ui.init_debug_ui(renderer.get_swap_chain_render_pass(), SwapChain::MAX_FRAMES_IN_FLIGHT);

	auto current_time = std::chrono::high_resolution_clock::now();

	performance_counter.start();
	while (!window.should_close())
	{
		glfwPollEvents();

		auto new_time = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration<float>(new_time - current_time);
		float frame_time = duration.count();
		Timestep timestep(duration);
		current_time = new_time;

		performance_counter.run(frame_time);

		input_system.handle_input(frame_time);

		float aspect = renderer.get_aspect_ratio();
		player_controller.get_camera().set_perspective_projection(glm::radians(90.f), aspect, 0.1f, 100.f);

		if (auto command_buffer = renderer.begin_frame())
		{
			int frame_index = renderer.get_frame_index();
			// update
			GlobalUbo ubo{};
			JointUbo joint_ubo{};

			auto& camera = player_controller.get_camera();
			ubo.projection = camera.get_projection_matrix();
			ubo.view = camera.get_view_matrix();
			ubo.inverse_view = camera.get_inverse_view_matrix();

			//gltf_model.animations->update(timestep, *gltf_model.skeleton, 7);
			//gltf_model.skeleton->Update();

			//for (int i = 0; i < gltf_model.skeleton->shader_data.final_joint_matrices.size(); i++)
			//{
			//	joint_ubo.joint_matrices[i] = gltf_model.skeleton->shader_data.final_joint_matrices[i];
			//}
			//joint_ubo.num_joints = gltf_model.skeleton->joints.size();

			point_light_system.update(object_manager_system.get_point_lights(), ubo, frame_time);

			uniform_buffers[frame_index]->write_to_buffer(&ubo);
			uniform_buffers[frame_index]->flush();

			//joint_buffers[frame_index]->write_to_buffer(&joint_ubo);
			//joint_buffers[frame_index]->flush();

			// render
			renderer.begin_swap_chain_render_pass(command_buffer);

			if (debug_ui.is_render_wireframe())
			{
				render_system.render_wireframe_game_objects(command_buffer, object_manager_system.get_game_objects(),
				                                            player_controller.get_camera(),
				                                            global_descriptor_sets[frame_index],
				                                            joint_descriptor_sets[frame_index]);
			}
			else
			{
				render_system.render_game_objects(
					command_buffer,
					object_manager_system.get_game_objects(),
					player_controller.get_camera(),
					global_descriptor_sets[frame_index],
					joint_descriptor_sets[frame_index],
					texture_manager_system.get_material_descriptor()->get_descriptor_set()
				);
			}

			point_light_system.render(command_buffer, object_manager_system.get_point_lights(),
			                          player_controller.get_camera(), global_descriptor_sets[frame_index]);

			if (input_system.get_is_menu_open())
			{
				debug_ui.draw(
					command_buffer,
					object_manager_system,
					player_controller,
					performance_counter
				);
			}
			else
			{
				object_manager_system.set_model_color(debug_ui.get_debug_object_id(), glm::vec3(0.0f, 0.0f, 0.0f));
			}

			renderer.end_swap_chain_render_pass(command_buffer);
			renderer.end_frame();
		}
	}
	performance_counter.stop();

	vkDeviceWaitIdle(device.get_logical_device());
}

void game_engine::Engine::load_game_objects(TextureManagerSystem& texture_manager_system)
{
	for (int i = 0; i < 10; i++)
	{
		auto rotate_light = glm::rotate(
			glm::mat4(1.f),
			(i * glm::two_pi<float>()) / 10.f,
			glm::vec3(0.f, -1.f, 0.f)
		);
		auto position = glm::vec3(rotate_light * glm::vec4(-5.f, -5.f, -5.f, 1.f));

		float hue = (i / static_cast<float>(10)) * glm::two_pi<float>();
		float r = 0.5f * (1.0f + cos(hue));
		float g = 0.5f * (1.0f + cos(hue + glm::two_pi<float>() / 3.0f));
		float b = 0.5f * (1.0f + cos(hue + 2.0f * glm::two_pi<float>() / 3.0f));
		auto color = glm::vec3(
			r, g, b
		);

		auto point_light = PointLightObject(
			1.0f,
			0.05f,
			color,
			position
		);
		object_manager_system.add_point_light(point_light);
	}

	auto gltf_model = std::make_shared<GltfModel>(device, "models/animated_model.gltf");
	auto game_object = GameObject(
		gltf_model,
		glm::vec3(0.0f),
		glm::vec3(0.0f),
		1.0f,
		glm::vec3(0.0f),
		"animated_model"
	);

	game_object.gltf_model->texture_id = texture_manager_system.load_texture(game_object.gltf_model->textures[0]);

	object_manager_system.add_game_object(game_object);

	auto gltf_model2 = std::make_shared<GltfModel>(device, "models/plane.gltf");
	auto game_object2 = GameObject(
		gltf_model2,
		glm::vec3(0.0f),
		glm::vec3(0.0f),
		1.0f,
		glm::vec3(0.0f),
		"plane"
	);
	game_object2.gltf_model->texture_id = texture_manager_system.load_texture(game_object2.gltf_model->textures[1]);
	object_manager_system.add_game_object(game_object2);
}
