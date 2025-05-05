#version 450

#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragUV;     // Add UV coordinates input

layout (location = 0) out vec4 color;

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

// Add texture sampler binding
layout(set = 2, binding = 0) uniform sampler2D texSampler[];

layout(push_constant) uniform Push {
	mat4 model_matrix;
	vec3 color;
	int texture_index;
} push;

void main()
{
	vec3 diffuse_light = ubo.ambient_light_color.xyz * ubo.ambient_light_color.w;
	vec3 specular_light = vec3(0.0);
	vec3 surface_normal = normalize(fragNormalWorld);

	vec3 camera_pos_world = ubo.inverse_view_matrix[3].xyz;
	vec3 view_direction = normalize(camera_pos_world - fragPosWorld);

	for (int i = 0; i < ubo.num_point_lights; i++)
	{
		PointLight light = ubo.point_lights[i];
		vec3 direction_to_light = light.position.xyz - fragPosWorld;
		float attenuation = 1.0 / dot(direction_to_light, direction_to_light);

		direction_to_light = normalize(direction_to_light);

		float cos_angle_incidence = max(dot(surface_normal, direction_to_light), 0.0);
		vec3 intensity = light.color.xyz * light.color.w * attenuation;

		diffuse_light += intensity * cos_angle_incidence;

		vec3 half_angle = normalize(view_direction + direction_to_light);
		float blinn_term = dot(surface_normal, half_angle);
		blinn_term = max(blinn_term, 0.0);
		blinn_term = pow(blinn_term, 32.0);
		specular_light += intensity * blinn_term;
	}

	// Base color from vertex or texture
	vec3 base_color = inColor;
	
	// Use texture if available
	if (push.texture_index >= -1) {
		base_color = texture(texSampler[push.texture_index], fragUV).rgb;
	}

	if(length(push.color) > 0.0)
	{
		color = vec4(push.color, 1.0);
	}
	else color = vec4(diffuse_light * base_color + specular_light * base_color, 1.0);
}