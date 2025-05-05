#include "point_light_object.h"

game_engine::PointLightObject::PointLightObject(float intensity, float radius, glm::vec3 color, glm::vec3 position)
	: light_intensity(intensity), light_radius(radius), light_color(color), position(position)
{
}