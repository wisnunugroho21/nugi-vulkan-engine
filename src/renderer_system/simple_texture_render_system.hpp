#pragma once

#include "../camera/camera.hpp"
#include "../device/device.hpp"
#include "../pipeline/pipeline.hpp"
#include "../game_object/game_object.hpp"
#include "../frame_info.hpp"
#include "../buffer/buffer.hpp"
#include "../descriptor/descriptor.hpp"
#include "../texture/texture.hpp"
#include "../globalUbo.hpp"

#include <memory>
#include <vector>

namespace nugiEngine {
	class EngineSimpleTextureRenderSystem {
		public:
			EngineSimpleTextureRenderSystem(EngineDevice& device, VkRenderPass renderPass, int objCount);
			~EngineSimpleTextureRenderSystem();

			EngineSimpleTextureRenderSystem(const EngineSimpleTextureRenderSystem&) = delete;
			EngineSimpleTextureRenderSystem& operator = (const EngineSimpleTextureRenderSystem&) = delete;

			VkDescriptorSet getBufferDescriptorSets(int index) { return this->bufferDescriptorSets[index]; }

			void writeUniformBuffer(int frameIndex, void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
			VkDescriptorSet setupTextureDescriptorSet(VkDescriptorImageInfo descImageInfo);
			void renderGameObjects(VkCommandBuffer commandBuffer, FrameInfo &frameInfo, std::vector<EngineGameObject> &gameObjects);

		private:
			void createPipelineLayout();
			void createPipeline(VkRenderPass renderPass);
			void createBuffers(unsigned long sizeUBO);
			void createDescriptor(int objCount);

			EngineDevice& appDevice;
			
			VkPipelineLayout pipelineLayout;
			std::unique_ptr<EnginePipeline> pipeline;

			std::unique_ptr<EngineDescriptorPool> bufferDescPool{};
			std::unique_ptr<EngineDescriptorSetLayout> bufferDescSetLayout{};
			std::vector<VkDescriptorSet> bufferDescriptorSets;

			std::unique_ptr<EngineDescriptorPool> textureDescPool{};
			std::unique_ptr<EngineDescriptorSetLayout> textureDescSetLayout{};

      std::vector<std::shared_ptr<EngineBuffer>> globalUboBuffers;
	};
}