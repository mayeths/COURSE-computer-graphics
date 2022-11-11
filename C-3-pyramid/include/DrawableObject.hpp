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

template <typename Derived>
class DrawableObject
{
   public:
    glm::vec3 position     = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    GLfloat rotationAngle  = 0; /* in radians */

    Derived& MoveTo(glm::vec3 newPosition)
    {
        this->position = newPosition;
        return (Derived&)*this;
    }
    Derived& MoveWith(glm::vec3 deltaPosition)
    {
        return this->MoveTo(this->position + deltaPosition);
    }

    Derived& RotateTo(glm::vec3 rotationAxis, GLfloat newAngle, bool angleIsDegrees = true)
    {
        if (angleIsDegrees) {
            newAngle = glm::radians(newAngle);
        }
        this->rotationAxis  = glm::normalize(rotationAxis);
        this->rotationAngle = newAngle;
        return (Derived&)*this;
    }
    Derived& RotateWith(glm::vec3 rotationAxis, GLfloat deltaAngle, bool angleIsDegrees = true)
    {
        if (angleIsDegrees) {
            deltaAngle     = glm::radians(deltaAngle);
            angleIsDegrees = false;
        }
        return this->RotateToRadians(rotationAxis, this->rotationAngle + deltaAngle, angleIsDegrees);
    }

    glm::mat4 GetModelMatrix()
    {
        glm::mat4 model = glm::mat4(1.0f);
        model           = glm::translate(glm::mat4(1.0f), this->position);
        model           = glm::rotate(model, this->rotationAngle, this->rotationAxis);
        return model;
    }

    // if the class will cointain some logic, so it must be refreshed at each game loop cycle by calling update.
    // Otherwise just don't override it.
    virtual void update(double now, double deltaUpdateTime){};
    virtual void render(double now, double deltaRenderTime, const glm::mat4 &view, const glm::mat4 &projection) = 0;
    virtual void GUIcallback(double deltaRenderTime){};
};
