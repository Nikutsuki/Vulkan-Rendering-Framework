#version 450
layout(triangles) in;
layout(line_strip, max_vertices = 4) out;

layout(location = 0) in vec3 fragColor[];
layout(location = 0) out vec3 gColor;

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
    // Emit the three edges of the triangle as a closed line loop
    for (int i = 0; i < 3; i++) {
        gColor = fragColor[i];
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    gColor = fragColor[0];
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    EndPrimitive();
}