#include "pipeline.hpp"

#include "../model/model.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace nugiEngine {
	EnginePipeline::Builder::Builder(EngineDevice& appDevice, VkPipelineLayout pipelineLayout, VkRenderPass renderPass) : appDevice{appDevice} {
		this->configInfo.pipelineLayout = pipelineLayout;
		this->configInfo.renderPass = renderPass;
	}

	EnginePipeline::Builder EnginePipeline::Builder::setDefault(const std::string& vertFilePath, const std::string& fragFilePath) {
		auto msaaSamples = this->appDevice.getMSAASamples();

		this->configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		this->configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		this->configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
		
		this->configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		this->configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
		this->configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		this->configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		this->configInfo.rasterizationInfo.lineWidth = 1.0f;
		this->configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		this->configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		this->configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
		this->configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
		this->configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
		this->configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional
		
		this->configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		this->configInfo.multisampleInfo.sampleShadingEnable = VK_TRUE;
		this->configInfo.multisampleInfo.rasterizationSamples = msaaSamples;
		this->configInfo.multisampleInfo.minSampleShading = 0.2f;           
		this->configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
		this->configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
		this->configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional
		
		this->configInfo.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		this->configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
		this->configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		this->configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		this->configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
		this->configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		this->configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		this->configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional
		
		this->configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		this->configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
		this->configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
		this->configInfo.colorBlendInfo.attachmentCount = 1;
		this->configInfo.colorBlendInfo.pAttachments = &this->configInfo.colorBlendAttachment;
		this->configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
		this->configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
		this->configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
		this->configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional
		
		this->configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		this->configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
		this->configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
		this->configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		this->configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		this->configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
		this->configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
		this->configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
		this->configInfo.depthStencilInfo.front = {};  // Optional
		this->configInfo.depthStencilInfo.back = {};   // Optional

		this->dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

		this->configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		this->configInfo.dynamicStateInfo.pDynamicStates = this->dynamicStates.data();
		this->configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(this->dynamicStates.size());
		this->configInfo.dynamicStateInfo.flags = 0;

		this->configInfo.bindingDescriptions = Vertex::getVertexBindingDescriptions();
		this->configInfo.attributeDescriptions = Vertex::getVertexAttributeDescriptions();

		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;

		auto vertCode = EnginePipeline::readFile(vertFilePath);
		auto fragCode = EnginePipeline::readFile(fragFilePath);

		EnginePipeline::createShaderModule(this->appDevice, vertCode, &vertShaderModule);
		EnginePipeline::createShaderModule(this->appDevice, fragCode, &fragShaderModule);

		VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
		vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexShaderStageInfo.module = vertShaderModule;
		vertexShaderStageInfo.pName = "main";
		vertexShaderStageInfo.flags = 0;
		vertexShaderStageInfo.pNext = nullptr;
		vertexShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
		fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentShaderStageInfo.module = fragShaderModule;
		fragmentShaderStageInfo.pName = "main";
		fragmentShaderStageInfo.flags = 0;
		fragmentShaderStageInfo.pNext = nullptr;
		fragmentShaderStageInfo.pSpecializationInfo = nullptr;

		this->shaderStagesInfo = { vertexShaderStageInfo, fragmentShaderStageInfo };
		this->configInfo.shaderStagesInfo = shaderStagesInfo;

		return *this;
	}

	EnginePipeline::Builder EnginePipeline::Builder::setSubpass(uint32_t subpass) {
		this->configInfo.subpass = subpass;
		return *this;
	}

	EnginePipeline::Builder EnginePipeline::Builder::setBindingDescriptions(std::vector<VkVertexInputBindingDescription> bindingDescriptions) {
		this->configInfo.bindingDescriptions = bindingDescriptions;
		return *this;
	}

	EnginePipeline::Builder EnginePipeline::Builder::setAttributeDescriptions (std::vector<VkVertexInputAttributeDescription> attributeDescriptions) {
		this->configInfo.attributeDescriptions = attributeDescriptions;
		return *this;
	}

	EnginePipeline::Builder EnginePipeline::Builder::setInputAssemblyInfo(VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo) {
		this->configInfo.inputAssemblyInfo = inputAssemblyInfo;
		return *this;
	}

	EnginePipeline::Builder EnginePipeline::Builder::setRasterizationInfo(VkPipelineRasterizationStateCreateInfo rasterizationInfo) {
		this->configInfo.rasterizationInfo = rasterizationInfo;
		return *this;
	}

	EnginePipeline::Builder EnginePipeline::Builder::setMultisampleInfo(VkPipelineMultisampleStateCreateInfo multisampleInfo) {
		this->configInfo.multisampleInfo = multisampleInfo;
		return *this;
	}

	EnginePipeline::Builder EnginePipeline::Builder::setColorBlendAttachment(VkPipelineColorBlendAttachmentState colorBlendAttachment) {
		this->configInfo.colorBlendAttachment = colorBlendAttachment;
		return *this;
	}

	EnginePipeline::Builder EnginePipeline::Builder::setColorBlendInfo(VkPipelineColorBlendStateCreateInfo colorBlendInfo) {
		this->configInfo.colorBlendInfo = colorBlendInfo;
		return *this;
	}

	EnginePipeline::Builder EnginePipeline::Builder::setDepthStencilInfo(VkPipelineDepthStencilStateCreateInfo depthStencilInfo) {
		this->configInfo.depthStencilInfo = depthStencilInfo;
		return *this;
	}

	EnginePipeline::Builder EnginePipeline::Builder::setDynamicStateInfo(VkPipelineDynamicStateCreateInfo dynamicStateInfo) {
		this->configInfo.dynamicStateInfo = dynamicStateInfo;
		return *this;
	}

	EnginePipeline::Builder EnginePipeline::Builder::setShaderStagesInfo(std::vector<VkPipelineShaderStageCreateInfo> shaderStagesInfo) {
		this->configInfo.shaderStagesInfo = shaderStagesInfo;
		return *this;
	}

	std::unique_ptr<EnginePipeline> EnginePipeline::Builder::build() {
		return std::make_unique<EnginePipeline>(
			this->appDevice,
			this->configInfo
		);
	}

	EnginePipeline::EnginePipeline(EngineDevice& device, const PipelineConfigInfo& configInfo) : engineDevice{device} {
		this->createGraphicPipeline(configInfo);
	}

	EnginePipeline::~EnginePipeline() {
		for (auto& shaderModule : this->shaderModules) {
			vkDestroyShaderModule(this->engineDevice.getLogicalDevice(), shaderModule, nullptr);
		}

		vkDestroyPipeline(this->engineDevice.getLogicalDevice(), this->graphicPipeline, nullptr);
	}

	std::vector<char> EnginePipeline::readFile(const std::string& filepath) {
		std::ifstream file{filepath, std::ios::ate | std::ios::binary};

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file");
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}

	void EnginePipeline::createGraphicPipeline(const PipelineConfigInfo& configInfo) {
		auto bindingDescriptions = configInfo.bindingDescriptions;
		auto attributeDescriptions = configInfo.attributeDescriptions;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

		VkPipelineViewportStateCreateInfo viewportInfo{};
		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportInfo.viewportCount = 1;
		viewportInfo.pViewports = nullptr;
		viewportInfo.scissorCount = 1;
		viewportInfo.pScissors = nullptr;

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(configInfo.shaderStagesInfo.size());
		pipelineInfo.pStages = configInfo.shaderStagesInfo.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportInfo;
		pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
		pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
		pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
		pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
		pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

		pipelineInfo.layout = configInfo.pipelineLayout;
		pipelineInfo.renderPass = configInfo.renderPass;
		pipelineInfo.subpass = configInfo.subpass;

		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(this->engineDevice.getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->graphicPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphic pipelines");
		}
		
		this->shaderModules.clear();
		for (auto& shaderStage : configInfo.shaderStagesInfo) {
			this->shaderModules.push_back(shaderStage.module);
		}
	}

	void EnginePipeline::createShaderModule(EngineDevice& appDevice, const std::vector<char>& code, VkShaderModule* shaderModule) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		if (vkCreateShaderModule(appDevice.getLogicalDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module");
		}
	}

	void EnginePipeline::bind(VkCommandBuffer commandBuffer) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicPipeline);
	}
} // namespace nugiEngine
