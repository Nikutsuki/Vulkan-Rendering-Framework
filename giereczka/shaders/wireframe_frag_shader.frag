#version 450
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout(location = 0) out vec4 outColor;

struct PointLight {
	vec4 position;
	vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection_matrix;
	mat4 view_matrix;
	mat4 inverse_view_matrix;
	vec4 ambient_light_color;
	PointLight point_lights[10];
	int num_point_lights;
} ubo;

layout(push_constant) uniform Push {
	mat4 model_matrix;
	vec3 color;
} push;

void main() {
    // You can output a fixed wireframe color or mix using gColor
    outColor = vec4(1.0, 1.0, 1.0, 1.0);
}