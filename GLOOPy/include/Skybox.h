#pragma once

#include "Texture.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include <vector>
#include <string>

class Skybox
{
    Buffer VBO;
    VertexArray VAO;
    Texture texture;
public:

    Skybox(std::vector<std::string> faces);

    void drawSkybox(Shader& skyboxShader);

};