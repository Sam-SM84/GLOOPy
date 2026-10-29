#pragma once

#include <glm/glm.hpp>
#include "Shader.h"

class Light
{
public:
	glm::vec3 direction;
	glm::vec3 color;

	Light(glm::vec3 dir, glm::vec3 _color);

	void apply(Shader& shader);
};

