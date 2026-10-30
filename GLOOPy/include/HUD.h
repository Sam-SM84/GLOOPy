#pragma once

#include "Texture.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "Shader.h"

class HUD
{
public:
	Texture texture;
	VertexArray VAO;
	Buffer EBO, VBO;
	Shader shader;
	glm::vec2 position;
	glm::vec2 size;

	HUD(Shader _shader, glm::vec2 _pos, glm::vec2 _size, const char* path = NULL);

	void setShader(glm::mat4 projection, bool image);

	void draw();
};