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

			VkFormat getSwapChainImageFormat() const { return this->swapChain->getSwapChainImageFormat(); }
			VkExtent2D getSwapChainExtent() const { return this->swapChain->getSwapChainExtent(); }
			float getAspectRatio() const { return this->swapChain->extentAspectRatio(); }
			int getSwapChainImageCount() const { return this->swapChain->imageCount(); }
			bool isFrameInProgress() const { return this->isFrameStarted; }
			
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
			void EngineRenderer::endFrame(std::shared_ptr<EngineCommandBuffer> commandBuffer);

			bool presentFrame(std::shared_ptr<EngineCommandBuffer> commandBuffer);

		private:
			void recreateSwapChain();
			void createGlobalBuffers(unsigned long sizeUBO, unsigned long sizeLightBuffer);
			void createGlobalUboDescriptor();
			void createSyncObjects(int imageCount);

			EngineWindow& appWindow;
			EngineDevice& appDevice;

			std::unique_ptr<EngineSwapChain> swapChain;
			std::vector<std::shared_ptr<EngineCommandBuffer>> commandBuffers;

			std::shared_ptr<EngineDescriptorPool> descriptorPool{};
			std::shared_ptr<EngineDescriptorSetLayout> globalDescSetLayout{};
			std::vector<std::shared_ptr<VkDescriptorSet>> globalDescriptorSets;

			std::vector<std::shared_ptr<EngineBuffer>> globalLightBuffers;
			std::vector<std::shared_ptr<EngineBuffer>> globalUniformBuffers;

			std::vector<VkSemaphore> imageAvailableSemaphores;
			std::vector<VkSemaphore> renderFinishedSemaphores;
			std::vector<VkFence> inFlightFences;
			std::vector<VkFence> imagesInFlight;
			size_t currentFrame = 0;

			uint32_t currentImageIndex = 0;
			int currentFrameIndex = 0;
			bool isFrameStarted = false;
	};
}