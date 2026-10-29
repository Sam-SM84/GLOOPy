#include "Skybox.h"

Skybox::Skybox(std::vector<std::string> faces)
{
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    VAO.bind();
    VBO.upload(GL_ARRAY_BUFFER, skyboxVertices, sizeof(skyboxVertices));
    Shader::defineArray(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    //glBindTexture(GL_TEXTURE_CUBE_MAP, texture.ID);
    texture.bind_cubeMap(0);

    for (unsigned int i = 0; i < faces.size(); i++)
    {
        texture.loadSkybox(faces[i].c_str(), i);
    }

    texture.setSkyboxParameter_filter(GL_LINEAR);
    texture.setSkyboxParameter_XY(GL_CLAMP_TO_EDGE);

}

void Skybox::drawSkybox(Shader& skyboxShader)
{

    glDepthFunc(GL_LEQUAL);



    VAO.bind();

    texture.bind_cubeMap(0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    skyboxShader.setInt("skybox", 0);
    VAO.unbind();
    glDepthFunc(GL_LESS);
}