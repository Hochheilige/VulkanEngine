#pragma once

#include <vulkan/vulkan.hpp>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <vector>

#include <Vertex.hpp>
#include <Buffer.hpp>

class Buffer;

struct Mesh {
	std::vector<Vertex> vertices;
	Buffer buffer;
};

struct MeshPushConstant {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 clip;
};