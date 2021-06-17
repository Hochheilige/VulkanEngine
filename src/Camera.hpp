#pragma once

#include <glm/mat4x4.hpp>

struct Camera {
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 clip;
};