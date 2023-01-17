#pragma once

#include "../command/command_buffer.hpp"
#include "../camera/camera.hpp"
#include "../device/device.hpp"
#include "../pipeline/compute_pipeline.hpp"
#include "../game_object/game_object.hpp"
#include "../frame_info.hpp"
#include "../buffer/buffer.hpp"
#include "../descriptor/descriptor.hpp"
#include "../globalUbo.hpp"

#include <memory>
#include <vector>

namespace nugiEngine {
	class EngineRayTracingComputeRender {
		public:
			EngineRayTracingComputeRender(EngineDevice& device, VkDescriptorSetLayout globalDescSetLayout, 
				EngineDescriptorPool &descriptorPool, uint32_t width, uint32_t height);
			~EngineRayTracingComputeRender();

			EngineRayTracingComputeRender(const EngineRayTracingComputeRender&) = delete;
			EngineRayTracingComputeRender& operator = (const EngineRayTracingComputeRender&) = delete;

			void render(std::shared_ptr<EngineCommandBuffer> commandBuffer, VkDescriptorSet &GlobalDescSet);

		private:
			void createPipelineLayout(VkDescriptorSetLayout globalDescSetLayouts);
			void createPipeline();

			EngineDevice& appDevice;
			
			VkPipelineLayout pipelineLayout;
			std::unique_ptr<EngineComputePipeline> pipeline;

			uint32_t width, height;
	};
}