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
			
			std::shared_ptr<EngineDescriptorPool> getDescriptorPool() const { return this->descriptorPool; }
			std::shared_ptr<EngineDescriptorSetLayout> getGlobalUboDescSetLayout() const { return this->globalUboDescSetLayout; }
			std::shared_ptr<VkDescriptorSet> getGlobalUboDescriptorSets(int index) const { return this->globalUboDescriptorSets[index]; }

			VkCommandBuffer getCommandBuffer() const { 
				assert(this->isFrameStarted && "cannot get command buffer when frame is not in progress");
				return this->commandBuffers->getBuffer(this->currentFrameIndex);
			}

			int getFrameIndex() {
				assert(this->isFrameStarted && "cannot get frame index when frame is not in progress");
				return this->currentFrameIndex;
			}

			void writeUniformBuffer(int frameIndex, void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

			VkCommandBuffer beginFrame();
			void endFrame(VkCommandBuffer commandBuffer);
			void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
			void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		private:
			void recreateSwapChain();
			void createGlobalUniformBuffers(unsigned long sizeUBO);
			void createGlobalUboDescriptor();

			EngineWindow& appWindow;
			EngineDevice& appDevice;

			std::unique_ptr<EngineCommandBuffer> commandBuffers;
			std::unique_ptr<EngineSwapChain> swapChain;

			std::shared_ptr<EngineDescriptorPool> descriptorPool{};
			std::shared_ptr<EngineDescriptorSetLayout> globalUboDescSetLayout{};
			std::vector<std::shared_ptr<VkDescriptorSet>> globalUboDescriptorSets;
			std::vector<std::shared_ptr<EngineBuffer>> globalUboBuffers;

			uint32_t currentImageIndex = 0;
			int currentFrameIndex = 0;
			bool isFrameStarted = false;
	};
}