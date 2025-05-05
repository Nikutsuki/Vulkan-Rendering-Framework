#pragma once

#include "pch.h"
#include "texture.h"

#define GLSL_HAS_DIFFUSE_MAP (0x1 << 0x0)
#define GLSL_HAS_NORMAL_MAP (0x1 << 0x1)
#define GLSL_HAS_ROUGHNESS_MAP (0x1 << 0x2)
#define GLSL_HAS_METALLIC_MAP (0x1 << 0x3)
#define GLSL_HAS_ROUGHNESS_METALLIC_MAP (0x1 << 0x4)
#define GLSL_HAS_EMISSIVE_COLOR (0x1 << 0x5)
#define GLSL_HAS_EMISSIVE_MAP (0x1 << 0x6)


namespace game_engine {
	class Material {
	public:
		enum TextureIndices
		{
			DIFFUSE_MAP_INDEX = 0,
			NORMAL_MAP_INDEX = 1,
			ROUGHNESS_MAP_INDEX = 2,
			METALLIC_MAP_INDEX = 3,
			ROUGHNESS_METALLIC_MAP_INDEX = 4,
			EMISSIVE_MAP_INDEX = 5,
			NUM_TEXTURES
		};

		typedef std::array<std::shared_ptr<Texture>, NUM_TEXTURES> MaterialTextures;

		enum MaterialFeatures
		{
			HAS_DIFFUSE_MAP = GLSL_HAS_DIFFUSE_MAP,
			HAS_NORMAL_MAP = GLSL_HAS_NORMAL_MAP,
			HAS_ROUGHNESS_MAP = GLSL_HAS_ROUGHNESS_MAP,
			HAS_METALLIC_MAP = GLSL_HAS_METALLIC_MAP,
			HAS_ROUGHNESS_METALLIC_MAP = GLSL_HAS_ROUGHNESS_METALLIC_MAP,
			HAS_EMISSIVE_COLOR = GLSL_HAS_EMISSIVE_COLOR,
			HAS_EMISSIVE_MAP = GLSL_HAS_EMISSIVE_MAP
		};

		struct PbrMaterialProperties
		{
			uint32_t material_features = 0;
			float roughness = 0.0f;
			float metallic = 0.0f;
			float normal_map_intensity = 1.0f;

			glm::vec4 diffuse_color = glm::vec4(1.0f);

			glm::vec3 emissive_color = glm::vec3(0.0f);
			float emissive_intensity = 1.0f;
		};

		PbrMaterialProperties pbr_material_properties;
		MaterialTextures material_textures;
	};
}