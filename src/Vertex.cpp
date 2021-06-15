#include <Vertex.hpp>

VertexInputDescription Vertex::GetVertexDescription() {
	vk::VertexInputBindingDescription mainBinding(
		0, sizeof(Vertex), vk::VertexInputRate::eVertex
	);

	vk::VertexInputAttributeDescription positionAttribute(
		0, // location
		0, // binding
		vk::Format::eR32G32B32Sfloat,
		offsetof(Vertex, position)
	);

	vk::VertexInputAttributeDescription normalAttribute(
		1, 0,
		vk::Format::eR32G32B32Sfloat,
		offsetof(Vertex, normal)
	);

	vk::VertexInputAttributeDescription colorAttribute(
		2, 0,
		vk::Format::eR32G32B32Sfloat,
		offsetof(Vertex, color)
	);

	VertexInputDescription description;
	description.bindings.push_back(mainBinding);
	description.attributes.push_back(positionAttribute);
	description.attributes.push_back(normalAttribute);
	description.attributes.push_back(colorAttribute);

	return description;
}