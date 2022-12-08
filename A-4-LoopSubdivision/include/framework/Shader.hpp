#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <glad/glad.h>

#include "util/log.h"

class Shader
{
public:
    unsigned int ID = 0;
public:
    std::string vertexPath;
    std::string fragmentPath;
    std::string geometryPath;
    std::string tessControlPath;
    std::string tessEvalPath;

    Shader() {}
    Shader(const std::string vertexPath, const std::string fragmentPath,
           const std::string tessControlPath = "", const std::string tessEvalPath = "",
           const std::string geometryPath = "")
    {
        this->vertexPath = vertexPath;
        this->fragmentPath = fragmentPath;
        this->geometryPath = geometryPath;
        this->tessControlPath = tessControlPath;
        this->tessEvalPath = tessEvalPath;
    }

    void Setup()
    {
        if (this->ID != 0)
            return;
        const char *vfile = vertexPath.c_str();
        const char *ffile = fragmentPath.c_str();
        const char *gfile = geometryPath.size() > 0 ? geometryPath.c_str() : "";
        const char *tcfile = tessControlPath.size() > 0 ? tessControlPath.c_str() : "";
        const char *tefile = tessEvalPath.size() > 0 ? tessEvalPath.c_str() : "";
        std::string vertexCode;
        std::string fragmentCode;
        std::string geometryCode;
        std::string tessControlCode;
        std::string tessEvalCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        std::ifstream gShaderFile;
        std::ifstream tcShaderFile;
        std::ifstream teShaderFile;
        std::stringstream vShaderStream;
        std::stringstream fShaderStream;
        std::stringstream gShaderStream;
        std::stringstream tcShaderStream;
        std::stringstream teShaderStream;
        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        tcShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        teShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try {
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            if (geometryPath.size() > 0) gShaderFile.open(geometryPath);
            if (tessControlPath.size() > 0) tcShaderFile.open(tessControlPath);
            if (tessEvalPath.size() > 0) teShaderFile.open(tessEvalPath);

            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            if (geometryPath.size() > 0) gShaderStream << gShaderFile.rdbuf();
            if (tessControlPath.size() > 0) tcShaderStream << tcShaderFile.rdbuf();
            if (tessEvalPath.size() > 0) teShaderStream << teShaderFile.rdbuf();

            vShaderFile.close();
            fShaderFile.close();
            if (geometryPath.size() > 0) gShaderFile.close();
            if (tessControlPath.size() > 0) tcShaderFile.close();
            if (tessEvalPath.size() > 0) teShaderFile.close();

            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
            if (geometryPath.size() > 0) geometryCode = gShaderStream.str();
            if (tessControlPath.size() > 0) tessControlCode = tcShaderStream.str();
            if (tessEvalPath.size() > 0) tessEvalCode = teShaderStream.str();
        } catch (std::ifstream::failure& e) {
            log_error("Shader(%s %s %s %s %s) was not successfully read: %s", vfile, ffile, gfile, tcfile, tefile, e.what());
            return;
        }
        const char *vShaderCode = vertexCode.c_str();
        const char *fShaderCode = fragmentCode.c_str();
        const char *gShaderCode = geometryCode.c_str();
        const char *tcShaderCode = tessControlCode.c_str();
        const char *teShaderCode = tessEvalCode.c_str();
        // vertex shader
        unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        if (!this->checkCompileErrors(vertex, "VERTEX", vfile)) {
            return;
        }
        // fragment shader
        unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        if (!this->checkCompileErrors(fragment, "FRAGMENT", ffile)) {
            return;
        }
        // if geometry shader is given, compile geometry shader
        unsigned int geometry = 0;
        if(geometryPath.size() > 0) {
            const char * gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            if (!this->checkCompileErrors(geometry, "GEOMETRY", gfile)) {
                return;
            }
        }
        // if tessellation shader is given, compile tessellation shader
        unsigned int tessControl = 0;
        if(tessControlPath.size() > 0) {
            const char * tcShaderCode = tessControlCode.c_str();
            tessControl = glCreateShader(GL_TESS_CONTROL_SHADER);
            glShaderSource(tessControl, 1, &tcShaderCode, NULL);
            glCompileShader(tessControl);
            if (!this->checkCompileErrors(tessControl, "TESS_CONTROL", tcfile)) {
                return;
            }
        }
        // if tessellation shader is given, compile tessellation shader
        unsigned int tessEval = 0;
        if(tessEvalPath.size() > 0) {
            const char * teShaderCode = tessEvalCode.c_str();
            tessEval = glCreateShader(GL_TESS_EVALUATION_SHADER);
            glShaderSource(tessEval, 1, &teShaderCode, NULL);
            glCompileShader(tessEval);
            if (!this->checkCompileErrors(tessEval, "TESS_EVALUATION", tefile)) {
                return;
            }
        }
        // shader Program
        this->ID = glCreateProgram();
        glAttachShader(this->ID, vertex);
        glAttachShader(this->ID, fragment);
        if (geometryPath.size() > 0) glAttachShader(this->ID, geometry);
        if (tessControlPath.size() > 0) glAttachShader(this->ID, tessControl);
        if (tessEvalPath.size() > 0) glAttachShader(this->ID, tessEval);
        glLinkProgram(this->ID);
        if (!this->checkCompileErrors(this->ID, "PROGRAM")) {
            return;
        }
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (geometryPath.size() > 0) glDeleteShader(geometry);
        // if (tessControlPath.size() > 0) glDeleteShader(tessControl);
        // if (tessEvalPath.size() > 0) glDeleteShader(tessEval);
    }

    ~Shader()
    {
        glDeleteProgram(this->ID);
    }

    void use()
    {
        glUseProgram(this->ID);
    }

    void setBool(const std::string &name, bool value) const
    {         
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }
    void setInt(const std::string &name, int value) const
    { 
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }
    void setFloat(const std::string &name, float value) const
    { 
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    }
    void setVec2(const std::string &name, const glm::vec2 &value) const
    { 
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec2(const std::string &name, float x, float y) const
    { 
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); 
    }
    void setVec3(const std::string &name, const glm::vec3 &value) const
    { 
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec3(const std::string &name, float x, float y, float z) const
    { 
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); 
    }
    void setVec4(const std::string &name, const glm::vec4 &value) const
    { 
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec4(const std::string &name, float x, float y, float z, float w) const
    { 
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
    }
    void setMat2(const std::string &name, const glm::mat2 &mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    void setMat3(const std::string &name, const glm::mat3 &mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    void setMat4(const std::string &name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

private:
    bool checkCompileErrors(unsigned int shader, std::string type, const char *file = nullptr)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                log_error("Shader compilation error (%s): %s", file, infoLog);
                return false;
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                log_error("Program link error: %s", infoLog);
                return false;
            }
        }
        return true;
    }
};
