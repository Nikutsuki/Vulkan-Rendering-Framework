#include "game_object.h"

game_engine::GameObject::GameObject() : color(glm::vec3(0.f)), name("")
{
	transform.translation = glm::vec3(0.f);
	transform.scale = 1.f;
	transform.rotation = glm::vec3(0.f);
}

game_engine::GameObject::GameObject(std::shared_ptr<GltfModel> gltf_model, glm::vec3 color, glm::vec3 translation, float scale, glm::vec3 rotation, std::string name) : gltf_model(gltf_model), color(color), name(name)
{
	transform.translation = translation;
	transform.scale = scale;
	transform.rotation = rotation;
}
