#include "ray_tracing_pipeline.hpp"

#include "../model/model.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace nugiEngine {
	EngineRayTracingPipeline::Builder::Builder(EngineDevice& appDevice, EngineDeviceProcedures &deviceProcedure, VkPipelineLayout pipelineLayout) 
		: appDevice{appDevice}, deviceProcedure{deviceProcedure} 
	{
		this->configInfo.pipelineLayout = pipelineLayout;
	}

	EngineRayTracingPipeline::Builder EngineRayTracingPipeline::Builder::setDefault(const std::string& raygenFilePath, const std::string& missFilePath, const std::string& closestHitFilePath) {
		auto msaaSamples = this->appDevice.getMSAASamples();

		this->dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

		this->configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		this->configInfo.dynamicStateInfo.pDynamicStates = this->dynamicStates.data();
		this->configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(this->dynamicStates.size());
		this->configInfo.dynamicStateInfo.flags = 0;

		VkShaderModule raygenShaderModule;
		VkShaderModule missShaderModule;
		VkShaderModule closestHitShaderModule;

		auto raygenCode = EngineRayTracingPipeline::readFile(raygenFilePath);
		auto missCode = EngineRayTracingPipeline::readFile(missFilePath);
		auto closestHitCode = EngineRayTracingPipeline::readFile(closestHitFilePath);

		EngineRayTracingPipeline::createShaderModule(this->appDevice, raygenCode, &raygenShaderModule);
		EngineRayTracingPipeline::createShaderModule(this->appDevice, missCode, &missShaderModule);
		EngineRayTracingPipeline::createShaderModule(this->appDevice, closestHitCode, &closestHitShaderModule);

		VkPipelineShaderStageCreateInfo raygenShaderStageInfo{};
		raygenShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		raygenShaderStageInfo.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		raygenShaderStageInfo.module = raygenShaderModule;
		raygenShaderStageInfo.pName = "main";
		raygenShaderStageInfo.flags = 0;
		raygenShaderStageInfo.pNext = nullptr;
		raygenShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo missShaderStageInfo{};
		missShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		missShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		missShaderStageInfo.module = missShaderModule;
		missShaderStageInfo.pName = "main";
		missShaderStageInfo.flags = 0;
		missShaderStageInfo.pNext = nullptr;
		missShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo closestHitShaderStageInfo{};
		closestHitShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		closestHitShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		closestHitShaderStageInfo.module = closestHitShaderModule;
		closestHitShaderStageInfo.pName = "main";
		closestHitShaderStageInfo.flags = 0;
		closestHitShaderStageInfo.pNext = nullptr;
		closestHitShaderStageInfo.pSpecializationInfo = nullptr;

		VkRayTracingShaderGroupCreateInfoKHR raygenShaderGroupInfo{};
		raygenShaderGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		raygenShaderGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		raygenShaderGroupInfo.generalShader = 0;
		raygenShaderGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
		raygenShaderGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
		raygenShaderGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;

		VkRayTracingShaderGroupCreateInfoKHR missShaderGroupInfo{};
		missShaderGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		missShaderGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		missShaderGroupInfo.generalShader = 1;
		missShaderGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
		missShaderGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
		missShaderGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;

		VkRayTracingShaderGroupCreateInfoKHR closestHitShaderGroupInfo{};
		closestHitShaderGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		closestHitShaderGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		closestHitShaderGroupInfo.generalShader = VK_SHADER_UNUSED_KHR;
		closestHitShaderGroupInfo.closestHitShader = 2;
		closestHitShaderGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
		closestHitShaderGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;

		this->shaderStagesInfo = { raygenShaderStageInfo, missShaderStageInfo, closestHitShaderStageInfo };
		this->shaderGroupsInfo = { raygenShaderGroupInfo, missShaderGroupInfo, closestHitShaderGroupInfo };

		this->configInfo.shaderStagesInfo = shaderStagesInfo;
		this->configInfo.shaderGroupsInfo = shaderGroupsInfo;

		return *this;
	}

	EngineRayTracingPipeline::Builder EngineRayTracingPipeline::Builder::setDynamicStateInfo(VkPipelineDynamicStateCreateInfo dynamicStateInfo) {
		this->configInfo.dynamicStateInfo = dynamicStateInfo;
		return *this;
	}

	EngineRayTracingPipeline::Builder EngineRayTracingPipeline::Builder::setShaderStagesInfo(std::vector<VkPipelineShaderStageCreateInfo> shaderStagesInfo) {
		this->configInfo.shaderStagesInfo = shaderStagesInfo;
		return *this;
	}

	EngineRayTracingPipeline::Builder EngineRayTracingPipeline::Builder::setShaderGroupsInfo(std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroupsInfo) {
		this->configInfo.shaderGroupsInfo = shaderGroupsInfo;
		return *this;
	}

	std::unique_ptr<EngineRayTracingPipeline> EngineRayTracingPipeline::Builder::build() {
		return std::make_unique<EngineRayTracingPipeline>(
			this->appDevice,
			this->deviceProcedure,
			this->configInfo
		);
	}

	EngineRayTracingPipeline::EngineRayTracingPipeline(EngineDevice& device, EngineDeviceProcedures &deviceProcedure, const PipelineConfigInfo& configInfo) 
		: engineDevice{device}, deviceProcedure{deviceProcedure} 
	{
		this->createGraphicPipeline(configInfo);
	}

	EngineRayTracingPipeline::~EngineRayTracingPipeline() {
		for (auto& shaderModule : this->shaderModules) {
			vkDestroyShaderModule(this->engineDevice.getLogicalDevice(), shaderModule, nullptr);
		}

		vkDestroyPipeline(this->engineDevice.getLogicalDevice(), this->graphicPipeline, nullptr);
	}

	std::vector<char> EngineRayTracingPipeline::readFile(const std::string& filepath) {
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

	uint32_t EngineRayTracingPipeline::aligned_size(uint32_t value, uint32_t alignment) { 
		return (value + alignment - 1) & ~(alignment - 1); 
	}

	void EngineRayTracingPipeline::createGraphicPipeline(const PipelineConfigInfo& configInfo) {
		VkRayTracingPipelineCreateInfoKHR pipelineInfo{};

		pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		pipelineInfo.stageCount = static_cast<uint32_t>(configInfo.shaderStagesInfo.size());
		pipelineInfo.pStages = configInfo.shaderStagesInfo.data();
		pipelineInfo.groupCount = static_cast<uint32_t>(configInfo.shaderGroupsInfo.size());
		pipelineInfo.pGroups = configInfo.shaderGroupsInfo.data();
		pipelineInfo.maxPipelineRayRecursionDepth = 1;
		pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;
		pipelineInfo.layout = configInfo.pipelineLayout;

		if (this->deviceProcedure.vkCreateRayTracingPipelinesKHR(this->engineDevice.getLogicalDevice(), 
			VK_NULL_HANDLE, VK_NULL_HANDLE, 1, 
			&pipelineInfo, 
			nullptr, 
			&this->graphicPipeline
		) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create ray tracing pipelines");
		}
		
		this->shaderModules.clear();
		for (auto& shaderStage : configInfo.shaderStagesInfo) {
			this->shaderModules.push_back(shaderStage.module);
		}
	}

	void EngineRayTracingPipeline::createShaderBindingTables(const PipelineConfigInfo& configInfo) {
    const uint32_t handleSize = this->engineDevice.getRayTracingProperties().shaderGroupHandleSize;
    const uint32_t handle_alignment = this->engineDevice.getRayTracingProperties().shaderGroupHandleAlignment;

    const uint32_t handleSizeAligned = EngineRayTracingPipeline::aligned_size(handleSize, handle_alignment);
    const uint32_t group_count = static_cast<uint32_t>(configInfo.shaderGroupsInfo.size());

    const uint32_t sbt_size = group_count * handleSizeAligned;

    this->raygenSBTBuffer = std::make_shared<EngineBuffer>(
      this->engineDevice,
			handleSizeAligned,
			group_count,
			VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    this->missgenSBTBuffer = std::make_shared<EngineBuffer>(
      this->engineDevice,
			handleSizeAligned,
			group_count,
			VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    
    this->hitgenSBTBuffer = std::make_shared<EngineBuffer>(
      this->engineDevice,
			handleSizeAligned,
			group_count,
			VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    // Copy the pipeline's shader handles into a host buffer
    std::vector<uint8_t> shaderHandleStorage(sbt_size);

    if (this->deviceProcedure.vkGetRayTracingShaderGroupHandlesKHR(
			this->engineDevice.getLogicalDevice(), 
			this->graphicPipeline, 
			0, group_count, sbt_size, 
			shaderHandleStorage.data()
		) != VK_SUCCESS) 
		{
      throw std::runtime_error("failed to create ray tracing shader group handler");
    }

    // Copy the shader handles from the host buffer to the binding tables
		this->raygenSBTBuffer->map();
		this->raygenSBTBuffer->writeToBuffer(shaderHandleStorage.data(), handleSize);
		this->raygenSBTBuffer->unmap();

		this->missgenSBTBuffer->map();
		this->missgenSBTBuffer->writeToBuffer(shaderHandleStorage.data() + handleSizeAligned, handleSize);
		this->missgenSBTBuffer->unmap();

		this->hitgenSBTBuffer->map();
		this->hitgenSBTBuffer->writeToBuffer(shaderHandleStorage.data() + handleSizeAligned * 2, handleSize);
		this->hitgenSBTBuffer->unmap();
  }

	void EngineRayTracingPipeline::createShaderModule(EngineDevice& appDevice, const std::vector<char>& code, VkShaderModule* shaderModule) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		if (vkCreateShaderModule(appDevice.getLogicalDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module");
		}
	}

	void EngineRayTracingPipeline::bind(VkCommandBuffer commandBuffer) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicPipeline);
	}
} // namespace nugiEngine
