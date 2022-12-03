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
			EngineSimpleTextureRenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalUboDescSetLayout, size_t objCount);
			~EngineSimpleTextureRenderSystem();

			EngineSimpleTextureRenderSystem(const EngineSimpleTextureRenderSystem&) = delete;
			EngineSimpleTextureRenderSystem& operator = (const EngineSimpleTextureRenderSystem&) = delete;
			
			VkDescriptorSet setupTextureDescriptorSet(VkDescriptorImageInfo descImageInfo);
			void render(VkCommandBuffer commandBuffer, VkDescriptorSet UBODescSet, FrameInfo &frameInfo, std::vector<EngineGameObject> &gameObjects);

		private:
			void createDescriptor(size_t objCount);
			void createPipelineLayout(VkDescriptorSetLayout globalUboDescSetLayout);
			void createPipeline(VkRenderPass renderPass);

			EngineDevice& appDevice;
			
			VkPipelineLayout pipelineLayout;
			std::unique_ptr<EnginePipeline> pipeline;

			std::unique_ptr<EngineDescriptorPool> textureDescPool{};
			std::unique_ptr<EngineDescriptorSetLayout> textureDescSetLayout{};
	};
}