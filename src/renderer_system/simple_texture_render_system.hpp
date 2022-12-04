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
			EngineSimpleTextureRenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalUboDescSetLayout);
			~EngineSimpleTextureRenderSystem();

			EngineSimpleTextureRenderSystem(const EngineSimpleTextureRenderSystem&) = delete;
			EngineSimpleTextureRenderSystem& operator = (const EngineSimpleTextureRenderSystem&) = delete;
			
			std::shared_ptr<VkDescriptorSet> setupTextureDescriptorSet(EngineDescriptorPool &descriptorPool, VkDescriptorImageInfo descImageInfo);
			void render(VkCommandBuffer commandBuffer, VkDescriptorSet UBODescSet, FrameInfo &frameInfo, std::vector<std::shared_ptr<EngineGameObject>> &gameObjects);

		private:
			void createDescriptor();
			void createPipelineLayout(VkDescriptorSetLayout globalUboDescSetLayout);
			void createPipeline(VkRenderPass renderPass);

			EngineDevice& appDevice;
			
			VkPipelineLayout pipelineLayout;
			std::unique_ptr<EnginePipeline> pipeline;

			std::shared_ptr<EngineDescriptorSetLayout> textureDescSetLayout{};
	};
}