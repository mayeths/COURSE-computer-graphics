#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "log.h"
#include "GUI.hpp"
#include "Camera.h"
#include "Snow.h"
#include "floor.h"

#include <iostream>

void key_callback(GLFWwindow* window,int key, int scancode,int action,int mode);
void do_movement();
void mouse_callback(GLFWwindow* window,double xpos,double ypos);
void scroll_callback(GLFWwindow* window,double xoffset,double yoffset);

GLfloat screenWidth = 1600;
GLfloat screenHeight = 1000;
Camera::Camera camera(glm::vec3(0.0f,0.0f,0.0f));
bool keys[1024];
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat lastX = 400,lastY = 300;
bool firstMouse = true;
string str_fps;
char c[8];
int FrameRate = 0;
int FrameCount = 0;
int timeLeft = 0;

int main(){
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


	GLFWwindow* window = glfwCreateWindow(screenWidth,screenHeight,"Learn OpenGL",NULL,NULL);
	if(window == NULL){
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window,mouse_callback);
	glfwSetScrollCallback(window,scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    Floor floor;
	Snow::Snow snow;
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

	while(!glfwWindowShouldClose(window)){
		glfwPollEvents();
		do_movement();
		//render
		glClearColor(0.0,0.0,0.0,1.0);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
		glm::mat4 projection(1.0f);
		glm::mat4 model(1.0f);
		glm::mat4 view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(45.0f),screenWidth/screenHeight,0.1f,2000.f);
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, position);
            floor.render(model,view,projection);
        }
		snow.Render(deltaTime,model,view,projection, floor.y);

		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();

	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode){
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window,GL_TRUE);
	if(action == GLFW_PRESS)
		keys[key] = true;
	else if(action == GLFW_RELEASE)
		keys[key] = false;
}

void do_movement(){
	GLfloat cameraSpeed = 5.0f*deltaTime;
	if(keys[GLFW_KEY_W])
		camera.ProcessKeyboard(Camera::FORWARD,deltaTime);
	if(keys[GLFW_KEY_S])
		camera.ProcessKeyboard(Camera::BACKWARD,deltaTime);
	if(keys[GLFW_KEY_A])
		camera.ProcessKeyboard(Camera::LEFTS,deltaTime);
	if(keys[GLFW_KEY_D])
		camera.ProcessKeyboard(Camera::RIGHTS,deltaTime);
}

void mouse_callback(GLFWwindow* window,double xpos,double ypos){
	if(firstMouse){
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	camera.ProcessMouseMovement(xoffset,yoffset);
}

void scroll_callback(GLFWwindow* window,double xoffset,double yoffset){
	camera.ProcessMouseScroll(yoffset);
}
