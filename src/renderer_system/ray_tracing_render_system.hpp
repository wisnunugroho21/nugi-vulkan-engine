#pragma once

#include "../command/command_buffer.hpp"
#include "../camera/camera.hpp"
#include "../device/device.hpp"
#include "../pipeline/ray_tracing_pipeline.hpp"
#include "../game_object/game_object.hpp"
#include "../frame_info.hpp"
#include "../buffer/buffer.hpp"
#include "../descriptor/descriptor.hpp"
#include "../acceleration_structure/top_level_acceleration_structure.hpp"
#include "../device/device_procedures.hpp"
#include "../globalUbo.hpp"

#include <memory>
#include <vector>

namespace nugiEngine {
	class EngineRayTracingRenderSystem {
		public:
			EngineRayTracingRenderSystem(EngineDevice& device, EngineDeviceProcedures &deviceProcedure, EngineDescriptorPool &descriptorPool, VkDescriptorSetLayout globalDescSetLayout, 
				std::vector<EngineTopLevelAccelerationStructure> topLevelAccelStructs, size_t imageCount, uint32_t width, uint32_t height);
			~EngineRayTracingRenderSystem();

			EngineRayTracingRenderSystem(const EngineRayTracingRenderSystem&) = delete;
			EngineRayTracingRenderSystem& operator = (const EngineRayTracingRenderSystem&) = delete;

			void render(std::shared_ptr<EngineCommandBuffer> commandBuffer, VkDescriptorSet &UBODescSet, 
				FrameInfo &frameInfo, std::shared_ptr<EngineImage> swapChainImage);

		private:
      void createStorageImage(size_t imageCount);
      void createDescriptor(EngineDescriptorPool &descriptorPool, size_t imageCount, std::vector<EngineTopLevelAccelerationStructure> topLevelAccelStructs);
			void createPipelineLayout(VkDescriptorSetLayout globalDescSetLayouts);
			void createPipeline();

			EngineDevice& appDevice;
			EngineDeviceProcedures &deviceProcedure;
			
			uint32_t width, height;
			
			VkPipelineLayout pipelineLayout;
			std::unique_ptr<EngineRayTracingPipeline> pipeline;

      std::vector<std::shared_ptr<EngineImage>> storageImages;
			std::vector<std::shared_ptr<VkDescriptorSet>> rayTracingDescSet;
      std::shared_ptr<EngineDescriptorSetLayout> rayTracingDescSetLayout{};
	};
}