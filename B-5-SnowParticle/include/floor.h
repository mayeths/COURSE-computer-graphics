#pragma once

#include <iostream>
#include <ctime>
#include <math.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include "shader.h"

class Floor {
private:

	GLuint VBO, VAO;
	Shader *shader;

public:
	Floor();

	~Floor();

	void render(glm::mat4 model, glm::mat4 view, glm::mat4 projection);

};