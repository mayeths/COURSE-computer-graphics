#pragma once

#include <iostream>
#include <ctime>
#include <math.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include "shader.h"
#include "texture.h"

class Floor {
private:

	GLuint VBO, VAO;
	Shader *shader;
public:
	CTexture texture;
	float y;
	Floor(float floorY = -3.0f);

	~Floor();

	void render(glm::mat4 model, glm::mat4 view, glm::mat4 projection);

};