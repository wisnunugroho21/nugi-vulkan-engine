#pragma once

#include "../camera/camera.hpp"
#include "../device/device.hpp"
#include "../pipeline/pipeline.hpp"
#include "../game_object/game_object.hpp"
#include "../frame_info.hpp"
#include "../buffer/buffer.hpp"
#include "../descriptor/descriptor.hpp"
#include "../globalUbo.hpp"

#include <memory>
#include <vector>

namespace nugiEngine {
	class EnginePointLightRenderSystem {
		public:
			EnginePointLightRenderSystem(EngineDevice& device, VkRenderPass renderPass);
			~EnginePointLightRenderSystem();

			EnginePointLightRenderSystem(const EnginePointLightRenderSystem&) = delete;
			EnginePointLightRenderSystem& operator = (const EnginePointLightRenderSystem&) = delete;

			VkDescriptorSet getGlobalDescriptorSets(int index) { return this->globalDescriptorSets[index]; }

			void writeUniformBuffer(int frameIndex, void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
			void render(VkCommandBuffer commandBuffer, FrameInfo &frameInfo, std::vector<EngineGameObject> &gameObjects);

		private:
			void createPipelineLayout();
			void createPipeline(VkRenderPass renderPass);
			void createBuffers(unsigned long sizeUBO);
			void createDescriptor();

			EngineDevice& appDevice;
			
			VkPipelineLayout pipelineLayout;
			std::unique_ptr<EnginePipeline> pipeline;

			std::unique_ptr<EngineDescriptorPool> globalPool{};
			std::unique_ptr<EngineDescriptorSetLayout> globalSetLayout{};
			std::vector<std::shared_ptr<EngineBuffer>> globalUboBuffers;
			std::vector<VkDescriptorSet> globalDescriptorSets;
	};
}