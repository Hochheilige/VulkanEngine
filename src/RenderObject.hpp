#pragma once

#include <vulkan/vulkan.hpp>

#include <glm/mat4x4.hpp>

#include <Mesh.hpp>

struct Material {
	vk::Pipeline pipeline;
	vk::PipelineLayout pipelineLayout;
};

struct RenderObject {
	Mesh* mesh = nullptr;
	Material* material = nullptr;
	glm::mat4x4 model;
};

