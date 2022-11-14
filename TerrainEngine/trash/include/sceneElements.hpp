#ifndef SCENELEMENTS_H
#define SCENELEMENTS_H

#include <glm/glm.hpp>
#include <random>
#include "Camera.hpp"
#include "buffers.hpp"

struct sceneElements {

	glm::vec3 lightPos, lightColor, lightDir, fogColor, seed;
	glm::mat4 projMatrix;
	Camera * cam;
	FrameBufferObject * sceneFBO;
	bool wireframe = false;
};

#endif
