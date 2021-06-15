#include <utils.hpp>

namespace utils {

	uint32_t findMemoryType(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags requirementsMask) {
		uint32_t typeIndex = uint32_t(~0);
		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			if ((typeBits & 1) &&
				((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask))
			{
				typeIndex = i;
				break;
			}
			typeBits >>= 1;
		}
		assert(typeIndex != uint32_t(~0));
		return typeIndex;
	}

	vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(vk::ShaderStageFlagBits stage, const vk::ShaderModule& shaderModule) {
		return vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			stage,
			shaderModule,
			"main"
		);
	}

	vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo() {
		return vk::PipelineVertexInputStateCreateInfo(
			vk::PipelineVertexInputStateCreateFlags(),
			0, 0
		);
	}

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo(vk::PrimitiveTopology topology) {
		return vk::PipelineInputAssemblyStateCreateInfo(
			vk::PipelineInputAssemblyStateCreateFlags(),
			topology,
			false
		);
	}

	vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo(vk::PolygonMode polygonMode) {
		return vk::PipelineRasterizationStateCreateInfo(
			vk::PipelineRasterizationStateCreateFlags(),
			false, false,
			polygonMode,
			vk::CullModeFlagBits::eBack,
			vk::FrontFace::eClockwise,
			false,
			0.0f,
			0.0f,
			0.0f,
			1.0f
		);
	}

	vk::PipelineMultisampleStateCreateInfo multisamplingStateCreateInfo() {
		return vk::PipelineMultisampleStateCreateInfo(
			vk::PipelineMultisampleStateCreateFlags(),
			vk::SampleCountFlagBits::e1,
			false,
			1.0f,
			nullptr,
			false, false
		);
	}

	vk::PipelineColorBlendAttachmentState colorBlendAttachmentState() {
		vk::ColorComponentFlags colorComponentFlags(
			vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
		);
		
		return vk::PipelineColorBlendAttachmentState(
			false,                   // blendEnable
			vk::BlendFactor::eZero,  // srcColorBlendFactor
			vk::BlendFactor::eZero,  // dstColorBlendFactor
			vk::BlendOp::eAdd,       // colorBlendOp
			vk::BlendFactor::eZero,  // srcAlphaBlendFactor
			vk::BlendFactor::eZero,  // dstAlphaBlendFactor
			vk::BlendOp::eAdd,       // alphaBlendOp
			colorComponentFlags      // colorWriteMask
		);
	}

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo() {
		return vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags()
		);
	}

	vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo(bool isDepthTest, bool isDepthWrite, vk::CompareOp compareOp) {
		vk::StencilOpState stencilOpState(
			vk::StencilOp::eKeep,
			vk::StencilOp::eKeep,
			vk::StencilOp::eKeep,
			vk::CompareOp::eAlways
		);

		return vk::PipelineDepthStencilStateCreateInfo(
			vk::PipelineDepthStencilStateCreateFlags(),
			isDepthTest, isDepthWrite,
			isDepthTest ? compareOp : vk::CompareOp::eAlways,
			false,
			false,
			stencilOpState,
			stencilOpState,
			0.0f, 1.0f
		);
	}

	vk::DescriptorSetLayoutBinding descriptorsetLayoutBinding(vk::DescriptorType type, vk::ShaderStageFlags stageFlags, uint32_t binding) {
		return vk::DescriptorSetLayoutBinding(
			binding,
			type, 
			stageFlags,
			{}
		);
	}

	vk::ShaderModule loadShaderModule(const char* filePath, const vk::Device& device) {
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);
		if (!file.is_open())
			throw std::runtime_error("cannot open shader file");

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

		file.seekg(0);
		file.read((char*)buffer.data(), fileSize);
		file.close();

		vk::ShaderModuleCreateInfo shaderModuleInfo(
			vk::ShaderModuleCreateFlags(), buffer
		);

		printf("Shader module %s successfully load\n", filePath);

		return device.createShaderModule(shaderModuleInfo);
	}

}