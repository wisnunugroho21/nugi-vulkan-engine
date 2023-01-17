#pragma once

#include <string>
#include <vector>
#include <memory>

#include "../device/device.hpp"

namespace nugiEngine {
	struct PipelineConfigInfo {
		VkPipelineLayout pipelineLayout = nullptr;

    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    VkPipeline basePipelineHandleInfo{};
    int32_t basePipelineIndex;
	};
	
	class EngineComputePipeline {
		public:
			class Builder {
				public:
					Builder(EngineDevice& appDevice, VkPipelineLayout pipelineLayout);
					Builder setDefault(const std::string& compFilePath);

					Builder setShaderStageInfo(VkPipelineShaderStageCreateInfo shaderStagesInfo);
          Builder setBasePipelineHandleInfo(VkPipeline basePipeline);
          Builder setBasePipelineIndex(int32_t basePipelineIndex);

					std::unique_ptr<EngineComputePipeline> build();

				private:
					PipelineConfigInfo configInfo{};
					EngineDevice& appDevice;
			};

			EngineComputePipeline(EngineDevice& device, const PipelineConfigInfo& configInfo);
			~EngineComputePipeline();

			EngineComputePipeline(const EngineComputePipeline&) = delete;
			EngineComputePipeline& operator = (const EngineDevice&) = delete;

			void bind(VkCommandBuffer commandBuffer);
			void dispatch(VkCommandBuffer commandBuffer, uint32_t xSize, uint32_t ySize, uint32_t zSize);

		private:
			EngineDevice& engineDevice;
			VkPipeline computePipeline;
      VkShaderModule shaderModule{};
			
			static std::vector<char> readFile(const std::string& filepath);
			static void createShaderModule(EngineDevice& appDevice, const std::vector<char>& code, VkShaderModule* shaderModule);
			void createGraphicPipeline(const PipelineConfigInfo& configInfo);
	};
}