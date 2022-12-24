#pragma once

#include "../command/command_buffer.hpp"
#include "../camera/camera.hpp"
#include "../device/device.hpp"
#include "../pipeline/ray_tracing_pipeline.hpp"
#include "../game_object/game_object.hpp"
#include "../frame_info.hpp"
#include "../buffer/buffer.hpp"
#include "../descriptor/descriptor.hpp"
#include "../globalUbo.hpp"

#include <memory>
#include <vector>

namespace nugiEngine {
	class EngineRayTracingRenderSystem {
		public:
			EngineRayTracingRenderSystem(EngineDevice& device, EngineDescriptorPool &descriptorPool, VkDescriptorSetLayout globalDescSetLayout, 
				std::vector<VkAccelerationStructureKHR> topLevelAccelStructs, int imageCount, int width, int height);
			~EngineRayTracingRenderSystem();

			EngineRayTracingRenderSystem(const EngineRayTracingRenderSystem&) = delete;
			EngineRayTracingRenderSystem& operator = (const EngineRayTracingRenderSystem&) = delete;

			void render(std::shared_ptr<EngineCommandBuffer> commandBuffer, VkDescriptorSet &UBODescSet, 
				FrameInfo &frameInfo, std::shared_ptr<EngineImage> swapChainImage);

		private:
      void createStorageImage(int imageCount);
      void createDescriptor(EngineDescriptorPool &descriptorPool, int imageCount, std::vector<VkAccelerationStructureKHR> topLevelAccelStructs);
			void createPipelineLayout(VkDescriptorSetLayout globalDescSetLayouts);
			void createPipeline();

			EngineDevice& appDevice;
			int width, height;
			
			VkPipelineLayout pipelineLayout;
			std::unique_ptr<EngineRayTracingPipeline> pipeline;

      std::vector<std::shared_ptr<EngineImage>> storageImages;
			std::vector<std::shared_ptr<VkDescriptorSet>> rayTracingDescSet;
      std::shared_ptr<EngineDescriptorSetLayout> rayTracingDescSetLayout{};
	};
}