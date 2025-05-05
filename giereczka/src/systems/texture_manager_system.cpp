//
// Created by Niku on 3/1/2025.
//

#include "systems/texture_manager_system.h"

game_engine::TextureManagerSystem::TextureManagerSystem(std::unique_ptr<MaterialDescriptor>& material_descriptor) : materials_descriptor{material_descriptor}
{
    for (int i = 1024; i >= 0; i--)
    {
        texture_indices.push_back(i);
    }
}

uint32_t game_engine::TextureManagerSystem::load_texture(
    std::shared_ptr<Texture>& texture
    )
{
    uint32_t texture_id = 0;

    if (!texture_indices.empty())
    {
        texture_id = texture_indices.back();
        texture_indices.pop_back();
    }
    else
    {
        texture_id = texture_count++;
    }

    materials_descriptor->write_texture(texture->get_image_view(), texture->get_sampler(), texture_id);

    textures[texture->get_file_name()] = texture_id;

    return texture_id;
}
