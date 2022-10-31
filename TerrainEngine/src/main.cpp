#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stdio.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include <Camera.hpp>

#include "Window.hpp"
#include "texture.hpp"
#include "log.h"

// #include "GUI.hpp"

namespace {
void errorCallback(int error, const char* description)
{ 
    log_error("GLFW error %d: %s\n", error, description);
}

GLFWwindow* initialize() {
    int glfwInitRes = glfwInit();
    if (!glfwInitRes) {
        log_fatal("Unable to initialize GLFW\n");
        return nullptr;
    }

    GLFWwindow* window = glfwCreateWindow(960, 540, "OpenGL MinGW Boilerplate", nullptr, nullptr);
    if (!window) {
        log_fatal("Unable to create GLFW window\n");
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    int gladInitRes = gladLoadGL();
    if (!gladInitRes) {
        log_fatal("Unable to initialize glad\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }

    return window;
}
}  // namespace

int main(int argc, char* argv[]) {
    glfwSetErrorCallback(errorCallback);

	glm::vec3 startPosition(0.0f, 800.0f, 0.0f);
	Camera camera(startPosition);

	int success;
	Window window(success, 1600, 900);
	if (!success) return -1;
	window.camera = &camera;

    GLint GLmajor = 0, GLminor = 0, GLrev = 0, flags = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &GLmajor);
    glGetIntegerv(GL_MINOR_VERSION, &GLminor);
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &GLrev);
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    log_info("OpenGL version: %d.%d.%d\n", GLmajor, GLminor, GLrev);

    int major, minor, rev;
    glfwGetVersion(&major, &minor, &rev);
    log_info("GLFW version: %d.%d.%d\n", major, minor, rev);

	while (window.continueLoop())
	{
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		window.swapBuffersAndPollEvents();
	}

    // GLFWwindow* window = initialize();
    // if (!window) {
    //     return 0;
    // }
	// // GUI gui(window);

    // // Set the clear color to a nice green
    // glClearColor(0.15f, 0.6f, 0.4f, 1.0f);

    // while (!glfwWindowShouldClose(window)) {
    //     glClear(GL_COLOR_BUFFER_BIT);

    //     glfwSwapBuffers(window);
    //     glfwPollEvents();
    // }

    // glfwDestroyWindow(window);
    // glfwTerminate();

    return 0;
}
