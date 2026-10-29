#include "Light.h"

Light::Light(glm::vec3 d, glm::vec3 c)
{
	direction = d;
	color = c;
}

void Light::apply(Shader& shader)
{
	shader.setVec3("lightDirection", direction);
	shader.setVec3("lightColor", color);

}