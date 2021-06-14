#pragma once

#include <vulkan/vulkan.hpp>

#include <glm/vec3.hpp>

#include <vector>

struct VertexInputDescription {
	std::vector<vk::VertexInputBindingDescription> bindings;
	std::vector<vk::VertexInputAttributeDescription> attributes;
};

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;

	static VertexInputDescription GetVertexDescription();
};