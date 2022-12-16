#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

class Object
{
public:
    glm::vec3 position     = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    GLfloat rotationAngle  = 0; /* in radians */
public:
    virtual void Update(double now, double lastUpdateTime, GLFWwindow *window) {}
    virtual void Render(double now, double lastRenderTime, const glm::mat4 &view, const glm::mat4 &projection) = 0;

    void MoveTo(glm::vec3 newPosition)
    {
        this->position = newPosition;
    }
    void MoveWith(glm::vec3 deltaPosition)
    {
        this->MoveTo(this->position + deltaPosition);
    }

    void RotateTo(glm::vec3 rotationAxis, GLfloat newAngle, bool angleIsDegrees = true)
    {
        if (angleIsDegrees) {
            newAngle = glm::radians(newAngle);
        }
        this->rotationAxis  = glm::normalize(rotationAxis);
        this->rotationAngle = newAngle;
    }
    void RotateWith(glm::vec3 rotationAxis, GLfloat deltaAngle, bool angleIsDegrees = true)
    {
        if (angleIsDegrees) {
            deltaAngle     = glm::radians(deltaAngle);
            angleIsDegrees = false;
        }
        this->RotateTo(rotationAxis, this->rotationAngle + deltaAngle, angleIsDegrees);
    }

    glm::mat4 GetModelMatrix()
    {
        glm::mat4 model = glm::mat4(1.0f);
        model           = glm::translate(glm::mat4(1.0f), this->position);
        model           = glm::rotate(model, this->rotationAngle, this->rotationAxis);
        return model;
    }

};
