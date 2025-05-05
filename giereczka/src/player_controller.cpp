#include "player_controller.h"

game_engine::PlayerController::PlayerController()
{
	camera = Camera();

	transform.translation = glm::vec3(0.f);
	transform.rotation = glm::vec3(0.f);

	camera.set_view_YXZ(transform.translation, transform.rotation);
}

void game_engine::PlayerController::move_player(glm::vec3 direction, float dt)
{
	float yaw = transform.rotation.y;
	glm::vec3 forward = glm::vec3(glm::sin(yaw), 0.f, glm::cos(yaw));
	glm::vec3 right = glm::vec3(glm::cos(yaw), 0.f, -glm::sin(yaw));
	glm::vec3 up = glm::vec3(0.f, -1.f, 0.f);

	glm::vec3 velocity = { 0.f, 0.f, 0.f };

	if (direction.x == -1)
	{
		velocity -= right;
	}
	if (direction.x == 1)
	{
		velocity += right;
	}
	if (direction.z == 1)
	{
		velocity += forward;
	}
	if (direction.z == -1)
	{
		velocity -= forward;
	}
	if (direction.y == 1)
	{
		velocity += up;
	}
	if (direction.y == -1)
	{
		velocity -= up;
	}

	if (glm::dot(velocity, velocity) > std::numeric_limits<float>::epsilon())
	{
		transform.translation += glm::normalize(velocity) * dt * movement_speed;
		camera.set_view_YXZ(transform.translation, transform.rotation);
	}
}

void game_engine::PlayerController::rotate_camera(double x_offset, double y_offset, float dt)
{

	glm::vec3 rotation = glm::vec3(0.f, 0.f, 0.f);

	rotation.y += x_offset * mouse_sensitivity / 100.f;
	rotation.x += y_offset * mouse_sensitivity / 100.f;

	if (glm::dot(rotation, rotation) > std::numeric_limits<float>::epsilon())
	{
		transform.rotation += rotation;
	}

	transform.rotation.x = glm::clamp(transform.rotation.x, -1.5f, 1.5f);
	transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());

	camera.set_view_YXZ(transform.translation, transform.rotation);
}
