#pragma once

#include <string>
#include <vector>
#include <memory>

#include "../device/device.hpp"
#include "../buffer/buffer.hpp"
#include "../device/device_procedures.hpp"

namespace nugiEngine {
	struct PipelineConfigInfo {
		VkPipelineLayout pipelineLayout = nullptr;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
		
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroupsInfo{};
		std::vector<VkPipelineShaderStageCreateInfo> shaderStagesInfo{};
	};
	
	class EngineRayTracingPipeline {
		public:
			class Builder {
				public:
					Builder(EngineDevice& appDevice, EngineDeviceProcedures &deviceProcedure, VkPipelineLayout pipelineLayout);

					std::vector<VkDynamicState> getDynamicStates() const { return this->dynamicStates; }
					std::vector<VkPipelineShaderStageCreateInfo> getShaderStagesInfo() const { return this->shaderStagesInfo; }

					Builder setDefault(const std::string& raygenFilePath, const std::string& missFilePath, const std::string& closestHitFilePath);

					Builder setDynamicStateInfo(VkPipelineDynamicStateCreateInfo dynamicStateInfo);
					Builder setShaderStagesInfo(std::vector<VkPipelineShaderStageCreateInfo> shaderStagesInfo);
					Builder setShaderGroupsInfo(std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroupsInfo);

					std::unique_ptr<EngineRayTracingPipeline> build();

				private:
					std::vector<VkDynamicState> dynamicStates{};
					std::vector<VkPipelineShaderStageCreateInfo> shaderStagesInfo{};
					std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroupsInfo{};

					PipelineConfigInfo configInfo{};

					EngineDevice& appDevice;
					EngineDeviceProcedures& deviceProcedure;
			};

			EngineRayTracingPipeline(EngineDevice& device, EngineDeviceProcedures &deviceProcedure, const PipelineConfigInfo& configInfo);
			~EngineRayTracingPipeline();

			EngineRayTracingPipeline(const EngineRayTracingPipeline&) = delete;
			EngineRayTracingPipeline& operator =(const EngineDevice&) = delete;

			std::shared_ptr<EngineBuffer> getRaygenSBTBuffer() const { return this->raygenSBTBuffer; }
			std::shared_ptr<EngineBuffer> getMissgenSBTBuffer() const { return this->missgenSBTBuffer; }
			std::shared_ptr<EngineBuffer> getHitgenSBTBuffer() const { return this->hitgenSBTBuffer; }

			void bind(VkCommandBuffer commandBuffer);
			static uint32_t aligned_size(uint32_t value, uint32_t alignment);

		private:
			EngineDevice& engineDevice;
			EngineDeviceProcedures& deviceProcedure;

			VkPipeline graphicPipeline;
			std::vector<VkShaderModule> shaderModules{};

			std::shared_ptr<EngineBuffer> raygenSBTBuffer;
      std::shared_ptr<EngineBuffer> missgenSBTBuffer;
      std::shared_ptr<EngineBuffer> hitgenSBTBuffer;
			
			static std::vector<char> readFile(const std::string& filepath);
			static void createShaderModule(EngineDevice& appDevice, const std::vector<char>& code, VkShaderModule* shaderModule);

			void createGraphicPipeline(const PipelineConfigInfo& configInfo);
			void createShaderBindingTables(const PipelineConfigInfo& configInfo);
	};
}