#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec3 tangent;
layout(location = 5) in ivec4 jointIds;
layout(location = 6) in vec4 weights;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec2 fragUV;

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

layout(set = 1, binding = 0) uniform JointUbo {
	mat4 joint_transforms[100];
	int num_joints;
} joint_ubo;

layout(push_constant) uniform Push {
	mat4 model_matrix;
	vec3 color;
	int texture_index;
} push;

const float AMBIENT = 0.02;

void main()
{
	vec4 animated_position = vec4(0.0f);
	mat4 joint_transform = mat4(0.0f);
	for(int i = 0; i < 4; ++i)
	{
		if(weights[i] == 0) continue;
		if(jointIds[i] >= joint_ubo.num_joints)
		{
			animated_position = vec4(position, 1.0f);
			joint_transform = mat4(1.0f);
			break;
		}

		vec4 local_position = joint_ubo.joint_transforms[jointIds[i]] * vec4(position, 1.0f);
		animated_position += local_position * weights[i];
		joint_transform += joint_ubo.joint_transforms[jointIds[i]] * weights[i];
	}

	vec4 position_world = push.model_matrix * vec4(position, 1.0);

	//vec4 position_world = normalize(push.model_matrix * animated_position);
	gl_Position = ubo.projection_matrix * ubo.view_matrix * position_world;

	fragNormalWorld = normalize(mat3(push.model_matrix) * normal);
	fragPosWorld = position_world.xyz;
	fragUV = uv;  // Pass UV coordinates to fragment shader
	fragColor = color;
}