#include "VulkanQuadRenderer.hpp"

// #include "Shaders.h"
#include "../utility/Utility.hpp"
#include "../window/glfw_window.hpp"

#include <vector>

///////////////////////////////////////////////////////////////////////////////
void VulkanQuadRenderer::ShutdownImpl()
{
	vkDestroyPipeline(this->device, this->pipeline, nullptr);
	vkDestroyPipelineLayout(this->device, this->pipelineLayout, nullptr);

	vkDestroyBuffer(this->device, this->vertexBuffer, nullptr);
	vkDestroyBuffer(this->device, this->indexBuffer, nullptr);
	vkFreeMemory(this->device, this->deviceMemory, nullptr);

	vkDestroyShaderModule(this->device, this->vertexShader, nullptr);
	vkDestroyShaderModule(this->device, this->fragmentShader, nullptr);

	VulkanRendererSystem::ShutdownImpl();
}

///////////////////////////////////////////////////////////////////////////////
void VulkanQuadRenderer::RenderImpl(VkCommandBuffer &commandBuffer)
{
	VulkanRendererSystem::RenderImpl(commandBuffer);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline);
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindIndexBuffer(commandBuffer, this->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &this->vertexBuffer, offsets);
	vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////
void VulkanQuadRenderer::InitializeImpl(VkCommandBuffer &uploadCommandBuffer, VkRenderPass &renderPass)
{
	VulkanRendererSystem::InitializeImpl(uploadCommandBuffer, renderPass);

	this->CreatePipelineStateObject("bin/shader/simple_shader.vert.spv", "bin/shader/simple_shader.frag.spv", renderPass);
	this->CreateMeshBuffers(uploadCommandBuffer);
}

///////////////////////////////////////////////////////////////////////////////
std::vector<MemoryTypeInfo> VulkanQuadRenderer::EnumerateHeaps(VkPhysicalDevice device)
{
	VkPhysicalDeviceMemoryProperties memoryProperties = {};
	vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

	std::vector<MemoryTypeInfo::Heap> heaps;

	for(uint32_t i = 0; i < memoryProperties.memoryHeapCount; ++i)
	{
		MemoryTypeInfo::Heap info;
		info.size = memoryProperties.memoryHeaps [i].size;
		info.deviceLocal =(memoryProperties.memoryHeaps [i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0;

		heaps.push_back(info);
	}

	std::vector<MemoryTypeInfo> result;

	for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		MemoryTypeInfo typeInfo;

		typeInfo.deviceLocal =(memoryProperties.memoryTypes [i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0;
		typeInfo.hostVisible =(memoryProperties.memoryTypes [i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
		typeInfo.hostCoherent =(memoryProperties.memoryTypes [i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;
		typeInfo.hostCached =(memoryProperties.memoryTypes [i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) != 0;
		typeInfo.lazilyAllocated =(memoryProperties.memoryTypes [i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) != 0;

		typeInfo.heap = heaps [memoryProperties.memoryTypes [i].heapIndex];

		typeInfo.index = static_cast<int>(i);

		result.push_back(typeInfo);
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////////
VkDeviceMemory VulkanQuadRenderer::AllocateMemory(const std::vector<MemoryTypeInfo>& memoryInfos,
	VkDevice device, const int size, bool* isHostCoherent)
{
	// We take the first HOST_VISIBLE memory
	for(auto& memoryInfo : memoryInfos)
	{
		if(memoryInfo.hostVisible)
		{
			VkMemoryAllocateInfo memoryAllocateInfo = {};
			memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memoryAllocateInfo.memoryTypeIndex = memoryInfo.index;
			memoryAllocateInfo.allocationSize = size;

			VkDeviceMemory deviceMemory;
			vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &deviceMemory);

			if(isHostCoherent != nullptr)
			{
				*isHostCoherent = memoryInfo.hostCoherent;
			}

			return deviceMemory;
		}
	}

	return VK_NULL_HANDLE;
}

///////////////////////////////////////////////////////////////////////////////
VkBuffer VulkanQuadRenderer::AllocateBuffer(VkDevice device, const int size,
	const VkBufferUsageFlagBits bits)
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = static_cast<uint32_t>(size);
	bufferCreateInfo.usage = bits;

	VkBuffer result;
	vkCreateBuffer(device, &bufferCreateInfo, nullptr, &result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
VkPipelineLayout VulkanQuadRenderer::CreatePipelineLayout(VkDevice device)
{
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	VkPipelineLayout result;
	vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr,&result);

	return result;
}

///////////////////////////////////////////////////////////////////////////////
VkShaderModule VulkanQuadRenderer::LoadShader(VkDevice device, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	shaderModuleCreateInfo.codeSize = code.size();

	VkShaderModule result;
	vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &result);

	return result;
}

///////////////////////////////////////////////////////////////////////////////
VkPipeline VulkanQuadRenderer::CreatePipeline(VkDevice device, VkRenderPass renderPass, VkPipelineLayout layout, 
	VkShaderModule vertexShader, VkShaderModule fragmentShader, VkExtent2D viewportSize)
{
	VkVertexInputBindingDescription vertexInputBindingDescription;
	vertexInputBindingDescription.binding = 0;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexInputBindingDescription.stride = sizeof(float) * 5;

	VkVertexInputAttributeDescription vertexInputAttributeDescription [2] = {};
	vertexInputAttributeDescription[0].binding = vertexInputBindingDescription.binding;
	vertexInputAttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeDescription[0].location = 0;
	vertexInputAttributeDescription[0].offset = 0;

	vertexInputAttributeDescription[1].binding = vertexInputBindingDescription.binding;
	vertexInputAttributeDescription[1].format = VK_FORMAT_R32G32_SFLOAT;
	vertexInputAttributeDescription[1].location = 1;
	vertexInputAttributeDescription[1].offset = sizeof(float) * 3;

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {};
	pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = std::extent<decltype(vertexInputAttributeDescription)>::value;
	pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescription;
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;

	VkPipelineInputAssemblyStateCreateInfo  pipelineInputAssemblyStateCreateInfo = {};
	pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = {};
	pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

	VkViewport viewport;
	viewport.height = static_cast<float>(viewportSize.height);
	viewport.width = static_cast<float>(viewportSize.width);
	viewport.x = 0;
	viewport.y = 0;
	viewport.minDepth = 0;
	viewport.maxDepth = 1;

	pipelineViewportStateCreateInfo.viewportCount = 1;
	pipelineViewportStateCreateInfo.pViewports = &viewport;

	VkRect2D rect;
	rect.extent = viewportSize;
	rect.offset.x = 0;
	rect.offset.y = 0;

	pipelineViewportStateCreateInfo.scissorCount = 1;
	pipelineViewportStateCreateInfo.pScissors = &rect;

	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = {};
	pipelineColorBlendAttachmentState.colorWriteMask = 0xF;
	pipelineColorBlendAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = {};
	pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

	pipelineColorBlendStateCreateInfo.attachmentCount = 1;
	pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;

	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = {};
	pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
	pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;

	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};
	pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_ALWAYS;
	pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
	pipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
	pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
	pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfo.front = pipelineDepthStencilStateCreateInfo.back;

	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = {};
	pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfos [2] = {};
	pipelineShaderStageCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineShaderStageCreateInfos[0].module = vertexShader;
	pipelineShaderStageCreateInfos[0].pName = "main";
	pipelineShaderStageCreateInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

	pipelineShaderStageCreateInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineShaderStageCreateInfos[1].module = fragmentShader;
	pipelineShaderStageCreateInfos[1].pName = "main";
	pipelineShaderStageCreateInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	graphicsPipelineCreateInfo.layout = layout;
	graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.renderPass = renderPass;
	graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
	graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos;
	graphicsPipelineCreateInfo.stageCount = 2;

	VkPipeline pipeline;
	vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline);

	return pipeline;
}

///////////////////////////////////////////////////////////////////////////////
void VulkanQuadRenderer::CreateMeshBuffers(VkCommandBuffer /*uploadCommandBuffer*/)
{
	struct Vertex
	{
		float position[3];
		float uv[2];
	};

	static const std::vector<Vertex> vertices[4] = {
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
	};

	static const int indices[6] = {
		0, 1, 2, 2, 3, 0
	};

	auto memoryHeaps = VulkanQuadRenderer::EnumerateHeaps(this->physicalDevice);
	this->indexBuffer = VulkanQuadRenderer::AllocateBuffer(this->device, sizeof(indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	this->vertexBuffer = VulkanQuadRenderer::AllocateBuffer(this->device, sizeof(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	VkMemoryRequirements vertexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(this->device, this->vertexBuffer, &vertexBufferMemoryRequirements);
	VkMemoryRequirements indexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(this->device, this->indexBuffer, &indexBufferMemoryRequirements);

	VkDeviceSize bufferSize = vertexBufferMemoryRequirements.size;
	VkDeviceSize indexBufferOffset = RoundToNextMultiple(bufferSize, indexBufferMemoryRequirements.alignment);

	bufferSize = indexBufferOffset + indexBufferMemoryRequirements.size;
	bool memoryIsHostCoherent = false;
	this->deviceMemory = VulkanQuadRenderer::AllocateMemory(memoryHeaps, this->device, static_cast<int>(bufferSize), &memoryIsHostCoherent);

	vkBindBufferMemory(this->device, this->vertexBuffer, this->deviceMemory, 0);
	vkBindBufferMemory(this->device, this->indexBuffer, this->deviceMemory, indexBufferOffset);

	void* mapping = nullptr;
	vkMapMemory(this->device, this->deviceMemory, 0, VK_WHOLE_SIZE, 0, &mapping);
	memcpy(mapping, vertices, sizeof(vertices));

	memcpy(static_cast<uint8_t*>(mapping) + indexBufferOffset, indices, sizeof(indices));

	if(!memoryIsHostCoherent)
	{
		VkMappedMemoryRange mappedMemoryRange = {};
		mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedMemoryRange.memory = this->deviceMemory;
		mappedMemoryRange.offset = 0;
		mappedMemoryRange.size = VK_WHOLE_SIZE;

		vkFlushMappedMemoryRanges(this->device, 1, &mappedMemoryRange);
	}

	vkUnmapMemory(this->device, this->deviceMemory);
}

///////////////////////////////////////////////////////////////////////////////
void VulkanQuadRenderer::CreatePipelineStateObject(const char* vertShaderFilename, const char* fragShaderFilename, VkRenderPass renderPass)
{
	auto vertCode = readFile(vertShaderFilename);
	auto fragCode = readFile(fragShaderFilename);

	this->vertexShader = LoadShader(this->device, vertCode);
	this->fragmentShader = LoadShader(this->device, fragCode);

	this->pipelineLayout = VulkanQuadRenderer::CreatePipelineLayout(this->device);
	VkExtent2D extent = { static_cast<uint32_t>(this->width), static_cast<uint32_t>(this->height) };
	this->pipeline = VulkanQuadRenderer::CreatePipeline(this->device, renderPass, this->pipelineLayout, this->vertexShader, this->fragmentShader, extent);
}
