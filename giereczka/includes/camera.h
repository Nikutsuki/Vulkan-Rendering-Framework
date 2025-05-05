#pragma once 

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <cassert>

namespace game_engine {
	class Camera {
	public:
		void set_ortho_projection(float left, float right, float bottom, float top, float near, float far);
		void set_perspective_projection(float aspect_ratio, float fov, float near, float far);

		void set_view_direction(
			const glm::vec3& from,
			const glm::vec3& to,
			const glm::vec3& up = glm::vec3{0.f, -1.f, 0.f}
		);

		void set_view_target(
			const glm::vec3& from,
			const glm::vec3& target,
			const glm::vec3& up = glm::vec3{ 0.f, -1.f, 0.f }
		);

		void set_view_YXZ(
			const glm::vec3 position,
			const glm::vec3 rotation
		);
		
		const glm::mat4& get_projection_matrix() const { return projection_matrix; }
		const glm::mat4& get_view_matrix() const { return view_matrix; }
		const glm::mat4& get_inverse_view_matrix() const { return inverse_view_matrix; }
	private:
		glm::mat4 projection_matrix{ 1.f };
		glm::mat4 view_matrix{ 1.f };
		glm::mat4 inverse_view_matrix{ 1.f };
	};
}