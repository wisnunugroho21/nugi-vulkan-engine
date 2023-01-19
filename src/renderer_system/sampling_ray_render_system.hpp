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
	class EngineSamplingRayRenderSystem {
		public:
			EngineSamplingRayRenderSystem(EngineDevice& device, std::shared_ptr<EngineDescriptorPool> descriptorPool, std::shared_ptr<EngineDescriptorSetLayout> traceRayDescLayout,
				std::vector<std::shared_ptr<EngineImage>> swapChainImages, uint32_t width, uint32_t height);
			~EngineSamplingRayRenderSystem();

			EngineSamplingRayRenderSystem(const EngineSamplingRayRenderSystem&) = delete;
			EngineSamplingRayRenderSystem& operator = (const EngineSamplingRayRenderSystem&) = delete;

			void render(std::shared_ptr<EngineCommandBuffer> commandBuffer, int imageIndex, std::shared_ptr<VkDescriptorSet> traceRayDescSet);

			bool prepareFrame(std::shared_ptr<EngineCommandBuffer> commandBuffer, int imageIndex);
			bool finishFrame(std::shared_ptr<EngineCommandBuffer> commandBuffer, int imageIndex);	
		
		private:
			void createPipelineLayout(std::shared_ptr<EngineDescriptorSetLayout> traceRayDescLayout);
			void createPipeline();

			void createDescriptor(std::shared_ptr<EngineDescriptorPool> descriptorPool);

			EngineDevice& appDevice;
			
			VkPipelineLayout pipelineLayout;
			std::unique_ptr<EngineComputePipeline> pipeline;

			std::shared_ptr<EngineDescriptorSetLayout> descSetLayout;
			std::vector<std::shared_ptr<VkDescriptorSet>> descriptorSets;

			std::vector<std::shared_ptr<EngineImage>> swapChainImages;

			uint32_t width, height;
	};
}