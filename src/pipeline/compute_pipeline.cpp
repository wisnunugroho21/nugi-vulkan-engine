#include "compute_pipeline.hpp"

#include "../model/model.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace nugiEngine {
	EngineComputePipeline::Builder::Builder(EngineDevice& appDevice, VkPipelineLayout pipelineLayout) : appDevice{appDevice} {
		this->configInfo.pipelineLayout = pipelineLayout;
	}

	EngineComputePipeline::Builder EngineComputePipeline::Builder::setDefault(const std::string& compFilePath) {
		VkShaderModule compShaderModule;

		auto compCode = EngineComputePipeline::readFile(compFilePath);

		EngineComputePipeline::createShaderModule(this->appDevice, compCode, &compShaderModule);

		VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
		computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStageInfo.module = compShaderModule;
		computeShaderStageInfo.pName = "main";
		computeShaderStageInfo.flags = 0;
		computeShaderStageInfo.pNext = nullptr;
		computeShaderStageInfo.pSpecializationInfo = nullptr;

		this->configInfo.shaderStageInfo = computeShaderStageInfo;
		return *this;
	}

  EngineComputePipeline::Builder EngineComputePipeline::Builder::setShaderStageInfo(VkPipelineShaderStageCreateInfo shaderStageInfo) {
		this->configInfo.shaderStageInfo = shaderStageInfo;
		return *this;
	}

  EngineComputePipeline::Builder EngineComputePipeline::Builder::setBasePipelineHandleInfo(VkPipeline basePipeline) {
    this->configInfo.basePipelineHandleInfo = basePipeline;
		return *this;
  }

  EngineComputePipeline::Builder EngineComputePipeline::Builder::setBasePipelineIndex(int32_t basePipelineIndex) {
    this->configInfo.basePipelineIndex = basePipelineIndex;
		return *this;
  }

	std::unique_ptr<EngineComputePipeline> EngineComputePipeline::Builder::build() {
		return std::make_unique<EngineComputePipeline>(
			this->appDevice,
			this->configInfo
		);
	}

	EngineComputePipeline::EngineComputePipeline(EngineDevice& device, const PipelineConfigInfo& configInfo) : engineDevice{device} {
		this->createGraphicPipeline(configInfo);
	}

	EngineComputePipeline::~EngineComputePipeline() {
		vkDestroyShaderModule(this->engineDevice.getLogicalDevice(), this->shaderModule, nullptr);
		vkDestroyPipeline(this->engineDevice.getLogicalDevice(), this->computePipeline, nullptr);
	}

	std::vector<char> EngineComputePipeline::readFile(const std::string& filepath) {
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

	void EngineComputePipeline::createGraphicPipeline(const PipelineConfigInfo& configInfo) {
    VkComputePipelineCreateInfo pipelineInfo{};

		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = configInfo.pipelineLayout;
		pipelineInfo.basePipelineIndex = configInfo.basePipelineIndex;
		pipelineInfo.basePipelineHandle = configInfo.basePipelineHandleInfo;
		pipelineInfo.stage = configInfo.shaderStageInfo;

		if (vkCreateComputePipelines(this->engineDevice.getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->computePipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create compute pipelines");
		}

    this->shaderModule = configInfo.shaderStageInfo.module;
	}

	void EngineComputePipeline::createShaderModule(EngineDevice& appDevice, const std::vector<char>& code, VkShaderModule* shaderModule) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		if (vkCreateShaderModule(appDevice.getLogicalDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module");
		}
	}

	void EngineComputePipeline::bind(VkCommandBuffer commandBuffer) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, this->computePipeline);
	}

	void EngineComputePipeline::dispatch(VkCommandBuffer commandBuffer, uint32_t xSize, uint32_t ySize, uint32_t zSize) {
		vkCmdDispatch(commandBuffer, xSize, ySize, zSize);
	}

} // namespace nugiEngine
