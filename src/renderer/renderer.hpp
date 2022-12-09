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
			std::shared_ptr<EngineDescriptorSetLayout> getglobalDescSetLayout() const { return this->globalDescSetLayout; }
			std::shared_ptr<VkDescriptorSet> getGlobalDescriptorSets(int index) const { return this->globalDescriptorSets[index]; }

			VkCommandBuffer getCommandBuffer() const { 
				assert(this->isFrameStarted && "cannot get command buffer when frame is not in progress");
				return this->commandBuffers[this->currentFrameIndex]->getCommandBuffer();
			}

			int getFrameIndex() {
				assert(this->isFrameStarted && "cannot get frame index when frame is not in progress");
				return this->currentFrameIndex;
			}

			void writeUniformBuffer(int frameIndex, void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
			void writeLightBuffer(int frameIndex, void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

			std::shared_ptr<EngineCommandBuffer> beginFrame();
			void endFrame(std::shared_ptr<EngineCommandBuffer> commandBuffer);
			void beginSwapChainRenderPass(std::shared_ptr<EngineCommandBuffer> commandBuffer);
			void endSwapChainRenderPass(std::shared_ptr<EngineCommandBuffer> commandBuffer);

		private:
			void recreateSwapChain();
			void createGlobalBuffers(unsigned long sizeUBO, unsigned long sizeLightBuffer);
			void createGlobalUboDescriptor();

			EngineWindow& appWindow;
			EngineDevice& appDevice;

			std::unique_ptr<EngineSwapChain> swapChain;
			std::vector<std::shared_ptr<EngineCommandBuffer>> commandBuffers;

			std::shared_ptr<EngineDescriptorPool> descriptorPool{};
			std::shared_ptr<EngineDescriptorSetLayout> globalDescSetLayout{};
			std::vector<std::shared_ptr<VkDescriptorSet>> globalDescriptorSets;

			std::vector<std::shared_ptr<EngineBuffer>> globalLightBuffers;
			std::vector<std::shared_ptr<EngineBuffer>> globalUniformBuffers;

			uint32_t currentImageIndex = 0;
			int currentFrameIndex = 0;
			bool isFrameStarted = false;
	};
}