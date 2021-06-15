#pragma once

#include <vulkan/vulkan.hpp>

#include <glm/vec3.hpp>

#include <vector>

#include <Vertex.hpp>
#include <Buffer.hpp>

class Buffer;

struct Mesh {
	std::vector<Vertex> vertices;
	Buffer buffer;
};