#pragma once

#include "../device/device.hpp"
#include "../pipeline/pipeline.hpp"
#include "../game_object/game_object.hpp"

#include <memory>
#include <vector>

namespace nugiEngine {
	class EngineSimpleRenderSystem
	{
		public:
			EngineSimpleRenderSystem(EngineDevice& device, VkRenderPass renderPass);
			~EngineSimpleRenderSystem();

			EngineSimpleRenderSystem(const EngineSimpleRenderSystem&) = delete;
			EngineSimpleRenderSystem& operator = (const EngineSimpleRenderSystem&) = delete;

			void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<EngineGameObject> &gameObjects);

		private:
			void createPipelineLayout();
			void createPipeline(VkRenderPass renderPass);

			EngineDevice& device;
			
			VkPipelineLayout pipelineLayout;
			std::unique_ptr<EnginePipeline> pipeline;
	};
}