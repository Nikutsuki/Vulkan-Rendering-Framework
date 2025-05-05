# CMakeList.txt : CMake project for giereczka, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable(
        giereczka
        "src/giereczka.cpp"
        "src/window.cpp"
        "src/engine.cpp"
        "src/pipelines/pipeline.cpp"
        "src/device.cpp"
        "src/swapchain.cpp"
        "src/model.cpp"
        "src/game_objects/game_object.cpp"
        "src/renderer.cpp"
        "src/systems/render_system.cpp"
        "src/camera.cpp"
        "src/topaz_input_system.cpp"
        "src/player_controller.cpp"
        "src/utils.cpp"
        "src/buffer.cpp"
        "src/descriptors.cpp"
        "src/systems/point_light_system.cpp"
        "src/game_objects/point_light_object.cpp"
        "src/buffers/global_ubo.cpp"
        "src/systems/object_manager_system.cpp"
        "src/debug_ui.cpp"
        "src/performance_counter.cpp"
        "src/skeletal_animations/gltf_model.cpp"
        "src/skeletal_animations/skeleton.cpp"
        "src/skeletal_animations/skeletal_animation.cpp"
        "src/skeletal_animations/skeletal_animations.cpp"
        "src/timestep.cpp"
        "src/texture.cpp"
        "src/material.cpp" "src/descriptors/material_descriptor.cpp"

        "src/systems/texture_manager_system.cpp"
        includes/systems/texture_manager_system.h
        src/pipelines/raytracing_pipeline.cpp
        includes/pipelines/raytracing_pipeline.h
        includes/pipelines/main_pipeline.h
        src/pipelines/main_pipeline.cpp
        includes/pipelines/light_pipeline.h
        src/pipelines/light_pipeline.cpp
        includes/systems/raytracing_render_system.h
        src/systems/raytracing_render_system.cpp
        src/pipelines/wireframe_pipeline.cpp
        includes/pipelines/wireframe_pipeline.h
        src/passes/shadow_pass.cpp
        includes/passes/shadow_pass.h)

include_directories(
        "includes"
        "includes/systems"
        "includes/game_objects"
        "includes/buffers"
        "external/tinyobjloader"
        "glfw-3.4/include"
        "N:/vulkan/Include"
)

target_precompile_headers(giereczka PRIVATE "includes/pch.h")

add_subdirectory("external\\glfw-3.4")

target_sources(giereczka PRIVATE
        external/imgui/imgui.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/backends/imgui_impl_glfw.cpp
        external/imgui/backends/imgui_impl_vulkan.cpp
        external/imgui/implot.cpp
        external/imgui/implot_items.cpp
)

target_include_directories(giereczka PRIVATE
        external/imgui
        external/imgui/backends
)

target_include_directories(giereczka PRIVATE
        external/tinygltf
)

find_package(Vulkan REQUIRED)

target_link_libraries(giereczka glfw Vulkan::Vulkan)

find_program(GLSLC glslc HINTS "N:\\vulkan\\Bin")

set(SHADER_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vert_shader.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/frag_shader.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/point_light.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/point_light.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/wireframe_frag_shader.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/wireframe_geom_shader.geom
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/wireframe_vert_shader.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/raytracing_compute_shader.comp
)

set(COMPILED_SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/compiled_shaders)
file(MAKE_DIRECTORY ${COMPILED_SHADER_DIR})

set(COMPILED_SHADERS)

foreach (SHADER ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)      # e.g., shader.vert
    get_filename_component(SHADER_WE ${SHADER} NAME_WE)       # e.g., shader

    # Choose the output filename based on the shader type (extension)
    get_filename_component(EXT ${SHADER} EXT)
    if (EXT STREQUAL ".vert")
        set(OUTPUT_FILE ${COMPILED_SHADER_DIR}/${SHADER_WE}.vert.spv)
    elseif (EXT STREQUAL ".frag")
        set(OUTPUT_FILE ${COMPILED_SHADER_DIR}/${SHADER_WE}.frag.spv)
    elseif (EXT STREQUAL ".geom")
        set(OUTPUT_FILE ${COMPILED_SHADER_DIR}/${SHADER_WE}.geom.spv)
    elseif (EXT STREQUAL ".comp")
        set(OUTPUT_FILE ${COMPILED_SHADER_DIR}/${SHADER_WE}.comp.spv)
    else ()
        message(FATAL_ERROR "Unknown shader extension for ${SHADER}")
    endif ()

    add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND ${GLSLC} ${SHADER} -o ${OUTPUT_FILE}
            DEPENDS ${SHADER}
            COMMENT "Compiling ${SHADER} -> ${OUTPUT_FILE}"
            VERBATIM
    )

    list(APPEND COMPILED_SHADERS ${OUTPUT_FILE})
endforeach ()

# Create a custom target to compile the shaders
add_custom_target(CompileShaders ALL DEPENDS ${COMPILED_SHADERS})

# Copy compiled shaders to the proper directory (e.g. a runtime shaders folder)
set(SHADER_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

add_custom_command(TARGET CompileShaders POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${COMPILED_SHADER_DIR} ${SHADER_OUTPUT_DIR}
        COMMENT "Copying compiled shaders to ${SHADER_OUTPUT_DIR}"
)

# Copy models to the proper directory (e.g. a runtime models folder)
add_custom_command(TARGET giereczka POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/models" "${CMAKE_CURRENT_BINARY_DIR}/models"
        COMMENT "Copying models directory from source to output directory"
)

# Ensure your executable depends on the shader compilation so they are built first
add_dependencies(giereczka CompileShaders)

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET giereczka PROPERTY CXX_STANDARD 20)
endif ()

# TODO: Add tests and install targets if needed.
