#pragma once

#include "../window/window.hpp"
#include "../device/device.hpp"
#include "../pipeline/pipeline.hpp"
#include "../swap_chain/swap_chain.hpp"
#include "../game_object/game_object.hpp"

#include <memory>
#include <vector>

namespace nugiEngine {
	class EngineApp
	{
		public:
			static constexpr int WIDTH = 800;
			static constexpr int HEIGHT = 600;

			EngineApp();
			~EngineApp();

			EngineApp(const EngineApp&) = delete;
			EngineApp& operator = (const EngineApp&) = delete;

			void run();

		private:
			void loadObjects();
			void createPipelineLayout();
			void createPipeline();
			void createCommandBuffers();
			void freeCommandBuffers();
			void drawFrame();
			void recreateSwapChain();
			void recordCommandBuffer(int imageIndex);
			void renderGameObjects(VkCommandBuffer commandBuffer);

			EngineWindow window{WIDTH, HEIGHT, "Testing vulkan"};
			EngineDevice device{window};
			std::unique_ptr<EngineSwapChain> swapChain;
			VkPipelineLayout pipelineLayout;
			std::unique_ptr<EnginePipeline> pipeline;
			std::vector<VkCommandBuffer> commandBuffers;
			std::vector<EngineGameObject> gameObjects;
	};
}