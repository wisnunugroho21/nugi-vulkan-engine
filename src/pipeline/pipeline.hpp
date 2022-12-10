#pragma once

#include <string>
#include <vector>

#include "../device/device.hpp"

namespace nugiEngine {
	struct PipelineConfigInfo {
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;

		std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
	};
	
	class EnginePipeline {
		public:
			class Builder {
				public:
					Builder(EngineDevice& appDevice, const std::string& vertFilePath, const std::string& fragFilePath, VkPipelineLayout pipelineLayout, VkRenderPass renderPass);

					Builder setDefault();

					Builder setSubpass(uint32_t subpass);
					Builder setBindingDescriptions(std::vector<VkVertexInputBindingDescription> bindingDescriptions);
					Builder setAttributeDescriptions (std::vector<VkVertexInputAttributeDescription> attributeDescriptions);

					Builder setInputAssemblyInfo(VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo);
					Builder setRasterizationInfo(VkPipelineRasterizationStateCreateInfo rasterizationInfo);
					Builder setMultisampleInfo(VkPipelineMultisampleStateCreateInfo multisampleInfo);
					Builder setColorBlendAttachment(VkPipelineColorBlendAttachmentState colorBlendAttachment);
					Builder setColorBlendInfo(VkPipelineColorBlendStateCreateInfo colorBlendInfo);
					Builder setDepthStencilInfo(VkPipelineDepthStencilStateCreateInfo depthStencilInfo);
					Builder setDynamicStateEnables(std::vector<VkDynamicState> dynamicStateEnables);
					Builder setDynamicStateInfo(VkPipelineDynamicStateCreateInfo dynamicStateInfo);

					std::unique_ptr<EnginePipeline> build();

				private:
					PipelineConfigInfo configInfo;
					EngineDevice& appDevice;
					const std::string& vertFilePath;
					const std::string& fragFilePath;
			};

			EnginePipeline(
				EngineDevice& device, 
				const std::string& vertFilePath, 
				const std::string& fragFilePath, 
				const PipelineConfigInfo& configInfo
			);
			~EnginePipeline();

			EnginePipeline(const EnginePipeline&) = delete;
			EnginePipeline& operator =(const EngineDevice&) = delete;

			void bind(VkCommandBuffer commandBuffer);

		private:
			EngineDevice& engineDevice;
			VkPipeline graphicPipeline;
			VkShaderModule vertShaderModule;
			VkShaderModule fragShaderModule;
			
			static std::vector<char> readFile(const std::string& filepath);
			void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
			void createGraphicPipeline(
				const std::string& vertFilePath, 
				const std::string& fragFilePath, 
				const PipelineConfigInfo& configInfo
			);
	};
}