#pragma once

#include "../window/window.hpp"
#include "../device/device.hpp"
#include "../swap_chain/swap_chain.hpp"
#include "../buffer/buffer.hpp"
#include "../descriptor/descriptor.hpp"
#include "../command/command_buffer.hpp"

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
			float getAspectRatio() const { return this->swapChain->extentAspectRatio(); }
			VkRenderPass getSwapChainRenderPass() const { return this->swapChain->getRenderPass(); }

			VkCommandBuffer getCommandBuffer() { 
				assert(isFrameStarted && "cannot get command buffer when frame is not in progress");
				return this->commandBuffers->getBuffer(this->currentFrameIndex);
			}

			int getFrameIndex() {
				assert(isFrameStarted && "cannot get frame index when frame is not in progress");
				return this->currentFrameIndex;
			}

			VkCommandBuffer beginFrame();
			void endFrame(VkCommandBuffer commandBuffer);
			void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
			void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		private:
			void recreateSwapChain();

			EngineWindow& appWindow;
			EngineDevice& appDevice;
			std::unique_ptr<EngineCommandBuffer> commandBuffers;
			std::unique_ptr<EngineSwapChain> swapChain;

			uint32_t currentImageIndex = 0;
			int currentFrameIndex = 0;
			bool isFrameStarted = false;
	};
}