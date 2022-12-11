#pragma once

#include "../command/command_buffer.hpp"
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
	class EngineTextureRenderSystem {
		public:
			EngineTextureRenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalDescSetLayout);
			~EngineTextureRenderSystem();

			EngineTextureRenderSystem(const EngineTextureRenderSystem&) = delete;
			EngineTextureRenderSystem& operator = (const EngineTextureRenderSystem&) = delete;
			
			std::shared_ptr<VkDescriptorSet> setupTextureDescriptorSet(EngineDescriptorPool &descriptorPool, VkDescriptorImageInfo descImageInfo);
			void render(std::shared_ptr<EngineCommandBuffer> commandBuffer, VkDescriptorSet &UBODescSet, FrameInfo &frameInfo, std::vector<std::shared_ptr<EngineGameObject>> &gameObjects);

		private:
			void createDescriptor();
			void createPipelineLayout(VkDescriptorSetLayout globalDescSetLayout);
			void createPipeline(VkRenderPass renderPass);

			EngineDevice& appDevice;
			
			VkPipelineLayout pipelineLayout;
			std::unique_ptr<EnginePipeline> pipeline;

			std::shared_ptr<EngineDescriptorSetLayout> textureDescSetLayout{};
	};
}