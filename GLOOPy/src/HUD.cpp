#include "HUD.h"

HUD::HUD(Shader _shader, glm::vec2 _pos, float _size, const char* path)
{
	shader = _shader;
	position = _pos;
	size = _size;

	float vertices[] =
	{
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
	};

	unsigned int indeces[]
	{
			0,1,3,
			2,3,1,
	};

	VAO.bind();

	VBO.upload(GL_ARRAY_BUFFER, vertices, sizeof(vertices));
	EBO.upload(GL_ELEMENT_ARRAY_BUFFER, indeces, sizeof(indeces));

	Shader::defineArray(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	VAO.unbind();

	if (path)
	{
		texture.loadImage(path);
		texture.setSkyboxParameter_XY(GL_REPEAT);
		texture.setSkyboxParameter_filter(GL_LINEAR);
	}
}

void HUD::setShader(glm::mat4 projection, bool image)
{
	shader.use();
	shader.setMat4("projection", projection);
	shader.setBool("image", image);
	shader.setInt("texture", 0);
	shader.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f));
	shader.setFloat("size", size);
	shader.setVec2("position", position);
}

void HUD::draw()
{
	glDisable(GL_DEPTH_TEST);
	VAO.bind();
	texture.bind(0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	VAO.unbind();
	glEnable(GL_DEPTH_TEST);
}
