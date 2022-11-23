#pragma once

#include "../camera/camera.hpp"
#include "../device/device.hpp"
#include "../pipeline/pipeline.hpp"
#include "../game_object/game_object.hpp"
#include "../frame_info.hpp"

#include <memory>
#include <vector>

namespace nugiEngine {
	class EngineSimpleRenderSystem
	{
		public:
			EngineSimpleRenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
			~EngineSimpleRenderSystem();

			EngineSimpleRenderSystem(const EngineSimpleRenderSystem&) = delete;
			EngineSimpleRenderSystem& operator = (const EngineSimpleRenderSystem&) = delete;

			void renderGameObjects(FrameInfo &frameInfo, std::vector<EngineGameObject> &gameObjects);

		private:
			void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
			void createPipeline(VkRenderPass renderPass);

			EngineDevice& device;
			
			VkPipelineLayout pipelineLayout;
			std::unique_ptr<EnginePipeline> pipeline;
	};
}