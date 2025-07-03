#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

class Shader
{
public:
    GLuint ID;
    Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
    {
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        // Open Shaders
        try {
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);

            if (!vShaderFile.is_open() || !fShaderFile.is_open()) {
                std::cerr << "Erro ao abrir arquivos shader." << std::endl;
                return;
            }

            std::stringstream vShaderStream, fShaderStream;
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            vShaderFile.close();
            fShaderFile.close();

            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();

            if (vertexCode.empty() || fragmentCode.empty()) {
                std::cerr << "Arquivos shader vazios." << std::endl;
                return;
            }

        } catch (std::ifstream::failure& e) {
            std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ\n" << e.what() << std::endl;
            return;
        }

        
        const GLchar* vShaderCode = vertexCode.c_str();
        const GLchar * fShaderCode = fragmentCode.c_str();

        GLuint vertex, fragment;
        GLint success;
        GLchar infoLog[1024];
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragment, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        this->ID = glCreateProgram();
        glAttachShader(this->ID, vertex);
        glAttachShader(this->ID, fragment);
        glLinkProgram(this->ID);
        glGetProgramiv(this->ID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(this->ID, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
            this->ID = 0;
        }
        glDeleteShader(vertex);
        glDeleteShader(fragment);

    }
    void Use()
    {
        if(this->ID != 0)
            glUseProgram(this->ID);
        else
            std::cerr << "Shader program inválido. glUseProgram não executado." << std::endl;
    }

    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(this->ID, name.c_str()), (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(this->ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(this->ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string& name, float v1, float v2, float v3) const
    {
        glUniform3f(glGetUniformLocation(this->ID, name.c_str()), v1, v2, v3);
    }
    // ------------------------------------------------------------------------
    // Novo método para setVec3 que aceita glm::vec3
    void setVec3(const std::string& name, const glm::vec3 &value) const
    {
        glUniform3fv(glGetUniformLocation(this->ID, name.c_str()), 1, glm::value_ptr(value));
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string& name, float v1, float v2, float v3, float v4) const
    {
        glUniform4f(glGetUniformLocation(this->ID, name.c_str()), v1, v2, v3,v4);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string& name, float *v) const
    {
        glUniformMatrix4fv(glGetUniformLocation(this->ID, name.c_str()), 1, GL_FALSE, v);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string& name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(this->ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }
};