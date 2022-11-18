#pragma once

#include "../window/window.hpp"
#include "../device/device.hpp"
#include "../pipeline/pipeline.hpp"
#include "../swap_chain/swap_chain.hpp"
#include "../model/model.hpp"

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
			void loadModels();
			void createPipelineLayout();
			void createPipeline();
			void createCommandBuffers();
			void freeCommandBuffers();
			void drawFrame();
			void recreateSwapChain();
			void recordCommandBuffer(int imageIndex);

			EngineWindow window{WIDTH, HEIGHT, "Testing vulkan"};
			EngineDevice device{window};
			std::unique_ptr<EngineSwapChain> swapChain;
			VkPipelineLayout pipelineLayout;
			std::unique_ptr<EnginePipeline> pipeline;
			std::vector<VkCommandBuffer> commandBuffers;
			std::unique_ptr<EngineModel> model;
	};
}