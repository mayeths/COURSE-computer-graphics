#pragma once

#if 1
#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

#include "Object/KeyboardListenerObject.hpp"

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
class Camera : KeyboardListenerObject
{
public:
    // Default camera values
    static inline const glm::vec3 ORIGIN = glm::vec3(0.0f, 0.0f, 0.0f);
    static inline const glm::vec3 BASE_X = glm::vec3(1.0f, 0.0f, 0.0f);
    static inline const glm::vec3 BASE_Y = glm::vec3(0.0f, 1.0f, 0.0f);
    static inline const glm::vec3 BASE_Z = glm::vec3(0.0f, 0.0f, 1.0f);
    static inline const glm::vec3 INIT_POSITION = Camera::ORIGIN;
    static inline const glm::vec3 INIT_FRONT = Camera::INIT_POSITION - Camera::BASE_Z;
    static inline const glm::vec3 INIT_WORLD_UP = Camera::BASE_Y;
    static inline const float INIT_MOVEMENT_SPEED =  100.0f;
    static inline const float INIT_MOUSE_SENSITIVITY =  0.08f;
    static inline const float INIT_ZOOM =  45.0f;
    static inline const float INIT_YAW = -90.0f;
    static inline const float INIT_PITCH =  0.0f;
    static inline const float INIT_ZOOM_SPRINGINESS = 10.0f; /* https://stackoverflow.com/a/10228863 */
    static inline const float INIT_MINIMUM_ZOOM = 25.0f;
    static inline const float INIT_MAXIMUM_ZOOM = 45.0f;

    // camera Attributes
    glm::vec3 Position = Camera::INIT_POSITION;
    // euler Angles
    float Yaw = Camera::INIT_YAW;
    float Pitch = Camera::INIT_PITCH;
    // camera options
    float MovementSpeed = Camera::INIT_MOVEMENT_SPEED;
    float MoveForwardScale = 1.4;
    float MoveHorizontalScale = 1.0;
    float MoveUpwardScale = 1.0;
    float MouseSensitivity = Camera::INIT_MOUSE_SENSITIVITY;
    float Zoom = Camera::INIT_ZOOM;
    float ZoomSpringiness = Camera::INIT_ZOOM_SPRINGINESS;
    float MinimumZoom = Camera::INIT_MINIMUM_ZOOM;
    float MaximumZoom = Camera::INIT_MAXIMUM_ZOOM;
    float MinRenderDistance = 0.1f;
    float MaxRenderDistance = 1e6f;
    double LastScrollTime = -100000.0f;
    double LastScrollY = 0.0f;
private:
    glm::vec3 _Front = glm::normalize(Camera::INIT_FRONT);
    glm::vec3 _Up;
    glm::vec3 _Right;
    glm::vec3 _WorldUp = Camera::INIT_WORLD_UP;

public:
    // constructor with vectors
    Camera(glm::vec3 initWorldUp = Camera::INIT_WORLD_UP)
    {
        this->updateCameraVectorsByEulerAngles();
    }

    Camera& setPosition(glm::vec3 newPosition)
    {
        this->Position = newPosition;
        return *this;
    }
    Camera& setLookAtTarget(glm::vec3 newTarget)
    {
        glm::vec3 direction = glm::normalize(newTarget - this->Position);
        this->Pitch = glm::degrees(asin(direction.y));
        this->Yaw = glm::degrees(atan2(direction.z, direction.x));
        this->updateCameraVectorsByEulerAngles();
        return *this;
    }
    Camera& setMovementSpeed(float newSpeed)
    {
        this->MovementSpeed = newSpeed;
        return *this;
    }
    Camera& setMoveForwardSpeed(float newScale)
    {
        this->MoveForwardScale = newScale;
        return *this;
    }
    Camera& setMoveHorizontalSpeed(float newScale)
    {
        this->MoveHorizontalScale = newScale;
        return *this;
    }
    Camera& setMoveUpwardSpeed(float newScale)
    {
        this->MoveUpwardScale = newScale;
        return *this;
    }
    Camera& setMouseSensitivity(float newMouseSensitivity)
    {
        this->MouseSensitivity = newMouseSensitivity;
        return *this;
    }
    Camera& setMinRenderDistance(float newMinRenderDistance)
    {
        this->MinRenderDistance = newMinRenderDistance;
        return *this;
    }
    Camera& setMaxRenderDistance(float newMaxRenderDistance)
    {
        this->MaxRenderDistance = newMaxRenderDistance;
        return *this;
    }


    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(this->Position, this->Position + this->_Front, this->_Up);
    }

    glm::mat4 GetProjectionMatrix(double lastRenderTime, double now, float screenRatio)
    {
        double delta = now - lastRenderTime;
        if (lastRenderTime - this->LastScrollTime <= 0.25f) {
            float range = (this->MaximumZoom - this->MinimumZoom);
            this->Zoom -= (float)this->LastScrollY * this->ZoomSpringiness * delta * range;
            if (this->Zoom < 15.0f)
                this->Zoom = 15.0f;
            if (this->Zoom > 45.0f)
                this->Zoom = 45.0f;
        }
        return glm::perspective(
            glm::radians(this->Zoom),
            screenRatio,
            this->MinRenderDistance, this->MaxRenderDistance
        );
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaUpdateTime)
    {
        float move = MovementSpeed * deltaUpdateTime;
        glm::vec3 frontInPlane = glm::normalize(glm::vec3(this->_Front.x, 0.0f, this->_Front.z));
        glm::vec3 rightInPlane = glm::normalize(glm::vec3(this->_Right.x, 0.0f, this->_Right.z));
        if (direction == FORWARD)
            this->Position += frontInPlane * this->MoveForwardScale * move;
        if (direction == BACKWARD)
            this->Position -= frontInPlane * this->MoveForwardScale * move;
        if (direction == LEFT)
            this->Position -= rightInPlane * this->MoveHorizontalScale * move;
        if (direction == RIGHT)
            this->Position += rightInPlane * this->MoveHorizontalScale * move;
        if (direction == UP)
            this->Position += this->_WorldUp * this->MoveUpwardScale * move;
        if (direction == DOWN)
            this->Position -= this->_WorldUp * this->MoveUpwardScale * move;
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
        // this->Zoom -= (float)yoffset;
        // if (this->Zoom < 25.0f)
        //     this->Zoom = 25.0f;
        // if (this->Zoom > 45.0f)
        //     this->Zoom = 45.0f;
        double maximumY = 5.0f;
        this->LastScrollY = yoffset / maximumY; /* G102 is 3.0f */
        this->LastScrollTime = glfwGetTime();
    }

    virtual std::map<int, KeyPriority> KeyboardRegister()
    {
        return {
            {GLFW_KEY_W, KeyPriority::Normal()},
            {GLFW_KEY_S, KeyPriority::Normal()},
            {GLFW_KEY_A, KeyPriority::Normal()},
            {GLFW_KEY_D, KeyPriority::Normal()},
            {GLFW_KEY_SPACE, KeyPriority::Normal()},
            {GLFW_KEY_LEFT_CONTROL, KeyPriority::Normal()},
            {GLFW_KEY_RIGHT_CONTROL, KeyPriority::Normal()},
        };
    }


    virtual bool KeyboardCallback(int key, KeyEvent event)
    {
        return false;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectorsByEulerAngles()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        front.y = sin(glm::radians(this->Pitch));
        front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        this->_Front = glm::normalize(front);
        this->_Right = glm::normalize(glm::cross(this->_Front, this->_WorldUp));
        this->_Up    = glm::normalize(glm::cross(this->_Right, this->_Front));
    }
};

#else

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Camera {

	enum Camera_Movement {
		FORWARD,
		BACKWARD,
		LEFTS,
		RIGHTS
	};

	const GLfloat YAW = -90.0f;
	const GLfloat PITCH = 0.0f;
	const GLfloat SPEED = 30.0f;
	const GLfloat SENSITIVITY = 0.25f;
	const GLfloat ZOOM = 45.0f;

	class Camera {
	public:
		glm::vec3 Position;
		glm::vec3 Front;
		glm::vec3 Up;
		glm::vec3 Right;
		glm::vec3 WorldUp;

		GLfloat Yaw;
		GLfloat Pitch;

		GLfloat MovementSpeed;
		GLfloat MouseSensitivity;
		GLfloat Zoom;

		Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
			glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW,GLfloat pitch = PITCH);

		Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ,
			GLfloat yaw, GLfloat pitch);

		glm::mat4 GetViewMatrix();

		void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime);

		void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true);

		void ProcessMouseScroll(GLfloat yoffset);

	private:

		void updateCameraVectors();
	};

}



namespace Camera {

	Camera::Camera(glm::vec3 position, glm::vec3 up, GLfloat yaw,GLfloat pitch) 
		:Front(glm::vec3(0.0f, -6.0f, -10.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY),
		Zoom(ZOOM) {
		this->Position = position;
		this->WorldUp = up;
		this->Yaw = yaw;
		this->Pitch = pitch;
		this->updateCameraVectors();
	}

	Camera::Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) :
		Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
		this->Position = glm::vec3(posX, posY, posZ);
		this->WorldUp = glm::vec3(upX, upY, upZ);
		this->Yaw = yaw;
		this->Pitch = pitch;
		this->updateCameraVectors();
	}

	glm::mat4 Camera::GetViewMatrix() {
		return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
	}

	void Camera::ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime) {
		GLfloat velocity = this->MovementSpeed * deltaTime;
		if (direction == FORWARD)
			this->Position += this->Front*velocity;
		if (direction == BACKWARD)
			this->Position -= this->Front*velocity;
		if (direction == LEFTS)
			this->Position -= this->Right*velocity;
		if (direction == RIGHTS)
			this->Position += this->Right*velocity;
		//this->Position.y = 0;
	}

	void Camera::ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch) {
		xoffset *= this->MouseSensitivity;
		yoffset *= this->MouseSensitivity;

		this->Yaw += xoffset;
		this->Pitch += yoffset;

		if (constrainPitch) {
			if (this->Pitch > 89.0f)
				this->Pitch = 89.0f;
			if (this->Pitch < -89.0f)
				this->Pitch = -89.0f;
		}
		this->updateCameraVectors();
	}

	void Camera::ProcessMouseScroll(GLfloat yoffset) {
		if (this->Zoom >= 1.0f && this->Zoom <= 45.0f)
			this->Zoom -= yoffset;
		if (this->Zoom <= 1.0f)
			this->Zoom = 1.0f;
		if (this->Zoom >= 45.0f)
			this->Zoom = 45.0f;
	}

	void Camera::updateCameraVectors() {
		glm::vec3 front;
		front.x = cos(glm::radians(this->Yaw))*cos(glm::radians(this->Pitch));
		front.y = sin(glm::radians(this->Pitch));
		front.z = sin(glm::radians(this->Yaw))*cos(glm::radians(this->Pitch));
		this->Front = glm::normalize(front);
		this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));
		this->Up = glm::normalize(glm::cross(this->Right, this->Front));
	}
}



#endif
