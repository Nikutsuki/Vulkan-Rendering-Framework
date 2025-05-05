#include "object_manager_system.h"

game_engine::ObjectManagerSystem::ObjectManagerSystem()
{
}

game_engine::ObjectManagerSystem::~ObjectManagerSystem()
{
}

void game_engine::ObjectManagerSystem::add_game_object(GameObject& game_object)
{
	for (auto& model : game_object.gltf_model->models) vertex_count += model->get_vertex_count();
	id_t id = assign_id();
	game_objects.emplace(id, std::move(game_object));
}

void game_engine::ObjectManagerSystem::add_point_light(PointLightObject& point_light)
{
	id_t id = assign_id();
	point_lights.emplace(id, std::move(point_light));
}

void game_engine::ObjectManagerSystem::move_game_object(id_t id, const glm::vec3& translation)
{
	game_objects[id].transform.translation = translation;
}

void game_engine::ObjectManagerSystem::scale_game_object(id_t id, float scale)
{
	game_objects[id].transform.scale = scale;
}

void game_engine::ObjectManagerSystem::rotate_game_object(id_t id, const glm::vec3& rotation)
{
	game_objects[id].transform.rotation = rotation;
}

void game_engine::ObjectManagerSystem::set_model_color(id_t id, const glm::vec3& color)
{
	if (game_objects.find(id) == game_objects.end()) return;
	game_objects[id].color = color;
}

void game_engine::ObjectManagerSystem::set_point_light_position(id_t id, const glm::vec3& position)
{
	point_lights[id].position = position;
}

void game_engine::ObjectManagerSystem::set_point_light_intensity(id_t id, float intensity)
{
	point_lights[id].light_intensity = intensity;
}

void game_engine::ObjectManagerSystem::set_point_light_radius(id_t id, float radius)
{
	point_lights[id].light_radius = radius;
}

void game_engine::ObjectManagerSystem::set_point_light_color(id_t id, const glm::vec3& color)
{
	point_lights[id].light_color = color;
}

void game_engine::ObjectManagerSystem::remove_game_object(id_t id)
{
	point_lights.erase(id);
}

void game_engine::ObjectManagerSystem::remove_point_light(id_t id)
{
	point_lights.erase(id);
}

game_engine::ObjectManagerSystem::id_t game_engine::ObjectManagerSystem::assign_id()
{
	return current_id++;
}
