#pragma once
#include<glad/glad.h>
#include <iostream>
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
class Shader
{
public:
    unsigned int ID;
    Shader(const char* vertexPath, const char* fragmentPath);
    Shader();

    void use();

    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string& name, bool value) const;

    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value) const;

    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) const;


    void setMat4(const std::string& name, glm::mat4 value) const;


    void setVec3(const std::string& name, glm::vec3 value) const;

    void setVec2(const std::string& name, glm::vec2 v) const;


    static void defineArray(GLuint index, GLuint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);


private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type);

};