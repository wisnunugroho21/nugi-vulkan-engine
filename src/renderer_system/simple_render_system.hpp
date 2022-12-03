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
	class EngineSimpleRenderSystem {
		public:
			EngineSimpleRenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalUboDescSetLayout);
			~EngineSimpleRenderSystem();

			EngineSimpleRenderSystem(const EngineSimpleRenderSystem&) = delete;
			EngineSimpleRenderSystem& operator = (const EngineSimpleRenderSystem&) = delete;

			void render(VkCommandBuffer commandBuffer, VkDescriptorSet UBODescSet, FrameInfo &frameInfo, std::vector<std::shared_ptr<EngineGameObject>> &gameObjects);

		private:
			void createPipelineLayout(VkDescriptorSetLayout globalUboDescSetLayouts);
			void createPipeline(VkRenderPass renderPass);

			EngineDevice& appDevice;
			
			VkPipelineLayout pipelineLayout;
			std::unique_ptr<EnginePipeline> pipeline;
	};
}