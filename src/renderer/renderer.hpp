#pragma once

#include "../window/window.hpp"
#include "../device/device.hpp"
#include "../swap_chain/swap_chain.hpp"
#include "../buffer/buffer.hpp"
#include "../descriptor/descriptor.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace nugiEngine {
	class EngineRenderer
	{
		public:
			EngineRenderer(EngineWindow& window, EngineDevice& device, unsigned long sizeUBO);
			~EngineRenderer();

			EngineRenderer(const EngineRenderer&) = delete;
			EngineRenderer& operator = (const EngineRenderer&) = delete;

			bool isFrameInProgress() const { return this->isFrameStarted; }
			float getAspectRatio() const { return this->swapChain->extentAspectRatio(); }
			VkRenderPass getSwapChainRenderPass() const { return this->swapChain->getRenderPass(); }
			VkDescriptorSetLayout getDescriptorSetLayout () { return this->globalSetLayout->getDescriptorSetLayout(); }
			VkDescriptorSet getGlobalDescriptorSets(int index) { return this->globalDescriptorSets[index]; }

			VkCommandBuffer getCommandBuffer() const { 
				assert(isFrameStarted && "cannot get command buffer when frame is not in progress");
				return this->commandBuffers[this->currentFrameIndex];
			}

			int getFrameIndex() {
				assert(isFrameStarted && "cannot get frame index when frame is not in progress");
				return this->currentFrameIndex;
			}

			VkCommandBuffer beginFrame();
			void writeUniformBuffer(int frameIndex, void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
			void endFrame(VkCommandBuffer commandBuffer);
			void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
			void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		private:
			void createCommandBuffers();
			void createBuffers(unsigned long sizeUBO);
			void freeCommandBuffers();
			void recreateSwapChain();

			EngineWindow& appWindow;
			EngineDevice& appDevice;
			std::unique_ptr<EngineSwapChain> swapChain;

			std::unique_ptr<EngineDescriptorPool> globalPool{};
			std::unique_ptr<EngineDescriptorSetLayout> globalSetLayout{};
			std::vector<std::shared_ptr<EngineBuffer>> globalUboBuffers;
			std::vector<VkDescriptorSet> globalDescriptorSets;

			std::vector<VkCommandBuffer> commandBuffers;

			uint32_t currentImageIndex = 0;
			int currentFrameIndex = 0;
			bool isFrameStarted = false;
	};
}