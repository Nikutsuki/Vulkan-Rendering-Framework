#pragma once

#include "game_object.h"
#include "point_light_object.h"

#include <unordered_map>

namespace game_engine {
	class ObjectManagerSystem {
	public:
		using id_t = unsigned int;
		using ObjectMap = std::unordered_map<id_t, GameObject>;
		using PointLightMap = std::unordered_map<id_t, PointLightObject>;

		ObjectManagerSystem();
		~ObjectManagerSystem();
		ObjectManagerSystem(const ObjectManagerSystem&) = delete;
		ObjectManagerSystem& operator=(const ObjectManagerSystem&) = delete;

		void add_game_object(GameObject& game_object);
		void add_point_light(PointLightObject& point_light);

		void remove_game_object(id_t id);
		void remove_point_light(id_t id);

		void move_game_object(id_t id, const glm::vec3& translation);
		void scale_game_object(id_t id, float scale);
		void rotate_game_object(id_t id, const glm::vec3& rotation);
		void set_model_color(id_t id, const glm::vec3& color);

		void set_point_light_position(id_t id, const glm::vec3& position);
		void set_point_light_intensity(id_t id, float intensity);
		void set_point_light_radius(id_t id, float radius);
		void set_point_light_color(id_t id, const glm::vec3& color);

		long long get_vertex_count() const { return vertex_count; }

		ObjectMap& get_game_objects() { return game_objects; };
		PointLightMap& get_point_lights() { return point_lights; };
	private:
		ObjectMap game_objects;
		PointLightMap point_lights;

		id_t current_id = 0;

		id_t assign_id();

		long long vertex_count = 0;
	};
}