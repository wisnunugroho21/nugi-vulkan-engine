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
			EnginePointLightRenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalUboDescSetLayout);
			~EnginePointLightRenderSystem();

			EnginePointLightRenderSystem(const EnginePointLightRenderSystem&) = delete;
			EnginePointLightRenderSystem& operator = (const EnginePointLightRenderSystem&) = delete;

			void update(FrameInfo &frameInfo, std::vector<std::shared_ptr<EngineGameObject>> &pointLightObjects,  GlobalUBO &ubo);
			void render(VkCommandBuffer commandBuffer, VkDescriptorSet UBODescSet, FrameInfo &frameInfo, std::vector<std::shared_ptr<EngineGameObject>> &pointLightObjects);

		private:
			void createPipelineLayout(VkDescriptorSetLayout globalUboDescSetLayout);
			void createPipeline(VkRenderPass renderPass);

			EngineDevice& appDevice;
			
			VkPipelineLayout pipelineLayout;
			std::unique_ptr<EnginePipeline> pipeline;
	};
}