#pragma once

#include <iostream>
#include <string>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include "stb_image.h"

using namespace std;

class CTexture {
public:
	GLuint textureID;

	CTexture();

	~CTexture();

	void loadTexture(string path, bool constrain = false);
};
