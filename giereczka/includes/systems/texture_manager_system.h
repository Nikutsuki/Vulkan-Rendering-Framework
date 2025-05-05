//
// Created by Niku on 3/1/2025.
//
#pragma once

#include "pch.h"

#include "device.h"
#include "texture.h"
#include "descriptors/material_descriptor.h"

namespace game_engine {
    class TextureManagerSystem {
    public:
        TextureManagerSystem(std::unique_ptr<MaterialDescriptor>& material_descriptor);

        uint32_t load_texture(std::shared_ptr<Texture>& texture);

        std::unique_ptr<MaterialDescriptor>& get_material_descriptor() { return materials_descriptor; }
    private:
        std::unique_ptr<MaterialDescriptor>& materials_descriptor;
        std::vector<uint32_t> texture_indices;
        std::unordered_map<std::string, uint32_t> textures;
        uint32_t texture_count = 0;
    };
};