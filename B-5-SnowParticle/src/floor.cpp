#include "floor.h"

Floor::Floor(float y) {
	this->y = y;
	float width = 1e5f;
	float vertices[] = {
		+width, this->y, +width,  1.0f*width, 1.0f*width,
		+width, this->y, -width,  1.0f*width, 0.0f*width,
		-width, this->y, -width,  0.0f*width, 0.0f*width,
		+width, this->y, +width,  1.0f*width, 1.0f*width,
		-width, this->y, -width,  0.0f*width, 0.0f*width,
		-width, this->y, +width,  0.0f*width, 1.0f*width,
        // -0.5f * width, -3.0f, -0.5f * width,  0.0f, 0.0f,
        // +0.5f * width, -3.0f, -0.5f * width,  1.0f, 0.0f,
        // +0.5f * width, -3.0f, -0.5f * width,  1.0f, 1.0f,
        // +0.5f * width, -3.0f, -0.5f * width,  1.0f, 1.0f,
        // -0.5f * width, -3.0f, -0.5f * width,  0.0f, 1.0f,
        // -0.5f * width, -3.0f, -0.5f * width,  0.0f, 0.0f,
	};
	shader = new Shader("./assets/Shaders/floor.vs", "./assets/Shaders/floor.fs");
	texture.loadTexture("./assets/Textures/floor.bmp", true);
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	shader->setInt("floorTexID", 0);
}

Floor::~Floor() {
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void Floor::render(glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
	shader->use();
	shader->setMat4("model", model);
	shader->setMat4("view", view);
	shader->setMat4("projection", projection);
	glBindVertexArray(VAO);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texture.textureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}