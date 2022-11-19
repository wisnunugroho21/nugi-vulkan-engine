#pragma once

#include "../window/window.hpp"
#include "../device/device.hpp"
#include "../swap_chain/swap_chain.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace nugiEngine {
	class EngineRenderer
	{
		public:
			EngineRenderer(EngineWindow& window, EngineDevice& device);
			~EngineRenderer();

			EngineRenderer(const EngineRenderer&) = delete;
			EngineRenderer& operator = (const EngineRenderer&) = delete;

			bool isFrameInProgress() const { return this->isFrameStarted; }
			VkRenderPass getSwapChainRenderPass() { return this->swapChain->getRenderPass(); }

			VkCommandBuffer getCommandBuffer() const { 
				assert(isFrameStarted && "cannot get command buffer when frame is not in progress");
				return this->commandBuffers[this->currentFrameIndex];
			}

			int getFrameIndex() {
				assert(isFrameStarted && "cannot get frame index when frame is not in progress");
				return this->currentFrameIndex;
			}

			VkCommandBuffer beginFrame();
			void endFrame();
			void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
			void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		private:
			void createCommandBuffers();
			void freeCommandBuffers();
			void recreateSwapChain();

			EngineWindow& appWindow;
			EngineDevice& appDevice;
			std::unique_ptr<EngineSwapChain> swapChain;
			std::vector<VkCommandBuffer> commandBuffers;

			uint32_t currentImageIndex = 0;
			int currentFrameIndex = 0;
			bool isFrameStarted = false;
	};
}