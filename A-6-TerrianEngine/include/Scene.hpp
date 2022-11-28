#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

class Scene
{
   public:
    glm::vec3 BaseX()
    {
        return glm::vec3(1.0f, 0.0f, 0.0f);
    }
    glm::vec3 BaseY()
    {
        return glm::vec3(0.0f, 1.0f, 0.0f);
    }
    glm::vec3 BaseZ()
    {
        return glm::vec3(0.0f, 0.0f, 1.0f);
    }
    glm::vec3 Right()
    {
        return +BaseX();
    }
    glm::vec3 Left()
    {
        return -BaseX();
    }
    glm::vec3 Up()
    {
        return +BaseY();
    }
    glm::vec3 Down()
    {
        return -BaseY();
    }
    glm::vec3 Front()
    {
        return -BaseZ();
    }
    glm::vec3 Behind()
    {
        return +BaseZ();
    }
};
