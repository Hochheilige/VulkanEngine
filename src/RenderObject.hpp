#pragma once

#include <vulkan/vulkan.hpp>

#include <Mesh.hpp>

struct Material {
	vk::Pipeline pipeline;
	vk::PipelineLayout pipelineLayout;
};

struct RenderObject {
	Mesh* mesh = nullptr;
	Material* material = nullptr;
	MeshPushConstant transform;
};

