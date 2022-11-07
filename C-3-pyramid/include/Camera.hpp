#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN,
};

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // Default camera values
    static inline const glm::vec3 INIT_POSITION = glm::vec3(0.0f, 0.0f, 0.0f);
    static inline const glm::vec3 INIT_FRONT = glm::vec3(0.0f, 0.0f, -1.0f);
    static inline const glm::vec3 INIT_WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);
    static inline const float INIT_MOVEMENT_SPEED =  2.5f;
    static inline const float INIT_MOUSE_SENSITIVITY =  0.1f;
    static inline const float INIT_ZOOM =  45.0f;
    static inline const float INIT_YAW = -90.0f;
    static inline const float INIT_PITCH =  0.0f;
    static inline const float INIT_ZOOM_SPRINGINESS = 35.0f; /* https://stackoverflow.com/a/10228863 */

    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    float ZoomSpringiness;
    double lastScrollPollTime = -100000.0f;
    double lastScrollPollYOffset = 0.0f;

public:
    // constructor with vectors
    Camera(glm::vec3 initWorldUp = Camera::INIT_WORLD_UP)
    {
        this->WorldUp = initWorldUp;
        this->Position = Camera::INIT_POSITION;
        this->Front = glm::normalize(Camera::INIT_FRONT);
        this->MovementSpeed = Camera::INIT_MOVEMENT_SPEED;
        this->MouseSensitivity = Camera::INIT_MOUSE_SENSITIVITY;
        this->Yaw = Camera::INIT_YAW;
        this->Pitch = Camera::INIT_PITCH;
        this->Zoom = Camera::INIT_ZOOM;
        this->ZoomSpringiness = Camera::INIT_ZOOM_SPRINGINESS;
        this->updateCameraVectorsByEulerAngles();
    }

    Camera& setPosition(glm::vec3 newPosition)
    {
        this->Position = newPosition;
        return *this;
    }
    Camera& setLookAtTarget(glm::vec3 newTarget)
    {
        glm::vec3 front = glm::normalize(newTarget - this->Position);
        this->Pitch = glm::degrees(asin(front.y));
        this->Yaw = glm::degrees(asin(front.z / cos(glm::radians(this->Pitch))));
        this->updateCameraVectorsByEulerAngles();
        // log_debug("pos (%.2f %.2f %.2f) front (%.2f %.2f %.2f) Yaw %.2f Pitch %.2f",
        //     this->Position.x, this->Position.y, this->Position.z,
        //     this->Front.x, this->Front.y, this->Front.z,
        //     this->Yaw, this->Pitch
        // );
        return *this;
    }
    Camera& setMovementSpeed(float newMovementSpeed)
    {
        this->MovementSpeed = newMovementSpeed;
        return *this;
    }
    Camera& setMouseSensitivity(float newMouseSensitivity)
    {
        this->MouseSensitivity = newMouseSensitivity;
        return *this;
    }


    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaUpdateTime)
    {
        float velocity = this->MovementSpeed * deltaUpdateTime;
        if (direction == FORWARD)
            this->Position += this->Front * velocity;
        if (direction == BACKWARD)
            this->Position -= this->Front * velocity;
        if (direction == LEFT)
            this->Position -= this->Right * velocity;
        if (direction == RIGHT)
            this->Position += this->Right * velocity;
        if (direction == UP)
            this->Position += this->Up * velocity;
        if (direction == DOWN)
            this->Position -= this->Up * velocity;
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= this->MouseSensitivity;
        yoffset *= this->MouseSensitivity;

        this->Yaw   += xoffset;
        this->Pitch += yoffset;

        if (constrainPitch) {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }
        this->updateCameraVectorsByEulerAngles();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 25.0f)
            Zoom = 25.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
        double maximumYOffset = 5.0f;
        this->lastScrollPollYOffset = yoffset / maximumYOffset; /* G102 is 3.0f */
        this->lastScrollPollTime = glfwGetTime();
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectorsByEulerAngles()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        front.y = sin(glm::radians(this->Pitch));
        front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        this->Front = glm::normalize(front);
        this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));
        this->Up    = glm::normalize(glm::cross(this->Right, this->Front));
    }
};
