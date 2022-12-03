#pragma once

#include <string>
#include <vector>

#include "../device/device.hpp"

namespace nugiEngine {
	struct PipelineConfigInfo {
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
	};
	
	class EnginePipeline
	{
		public:
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
			static void defaultPipelineConfigInfo(EngineDevice &appDevice, PipelineConfigInfo& configInfo);

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