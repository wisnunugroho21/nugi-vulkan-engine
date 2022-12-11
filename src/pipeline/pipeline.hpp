#pragma once

#include <string>
#include <vector>
#include <memory>

#include "../device/device.hpp"

namespace nugiEngine {
	struct PipelineConfigInfo {
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;

		std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
		VkPipelineMultisampleStateCreateInfo multisampleInfo{};
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
		VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
		std::vector<VkPipelineShaderStageCreateInfo> shaderStagesInfo{};
	};
	
	class EnginePipeline {
		public:
			class Builder {
				public:
					Builder(EngineDevice& appDevice, VkPipelineLayout pipelineLayout, VkRenderPass renderPass);

					std::vector<VkDynamicState> getDynamicStates() const { return this->dynamicStates; }
					std::vector<VkPipelineShaderStageCreateInfo> getShaderStagesInfo() const { return this->shaderStagesInfo; }

					Builder setDefault(const std::string& vertFilePath, const std::string& fragFilePath);

					Builder setSubpass(uint32_t subpass);
					Builder setBindingDescriptions(std::vector<VkVertexInputBindingDescription> bindingDescriptions);
					Builder setAttributeDescriptions (std::vector<VkVertexInputAttributeDescription> attributeDescriptions);

					Builder setInputAssemblyInfo(VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo);
					Builder setRasterizationInfo(VkPipelineRasterizationStateCreateInfo rasterizationInfo);
					Builder setMultisampleInfo(VkPipelineMultisampleStateCreateInfo multisampleInfo);
					Builder setColorBlendAttachment(VkPipelineColorBlendAttachmentState colorBlendAttachment);
					Builder setColorBlendInfo(VkPipelineColorBlendStateCreateInfo colorBlendInfo);
					Builder setDepthStencilInfo(VkPipelineDepthStencilStateCreateInfo depthStencilInfo);
					Builder setDynamicStateInfo(VkPipelineDynamicStateCreateInfo dynamicStateInfo);
					Builder setShaderStagesInfo(std::vector<VkPipelineShaderStageCreateInfo> shaderStagesInfo);

					std::unique_ptr<EnginePipeline> build();

				private:
					std::vector<VkDynamicState> dynamicStates{};
					std::vector<VkPipelineShaderStageCreateInfo> shaderStagesInfo{};
					PipelineConfigInfo configInfo{};
					
					EngineDevice& appDevice;
			};

			EnginePipeline(EngineDevice& device, const PipelineConfigInfo& configInfo);
			~EnginePipeline();

			EnginePipeline(const EnginePipeline&) = delete;
			EnginePipeline& operator =(const EngineDevice&) = delete;

			void bind(VkCommandBuffer commandBuffer);

		private:
			EngineDevice& engineDevice;
			VkPipeline graphicPipeline;
			std::vector<VkShaderModule> shaderModules{};
			
			static std::vector<char> readFile(const std::string& filepath);
			static void createShaderModule(EngineDevice& appDevice, const std::vector<char>& code, VkShaderModule* shaderModule);
			void createGraphicPipeline(const PipelineConfigInfo& configInfo);
	};
}