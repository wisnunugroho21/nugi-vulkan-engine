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
	class EngineRasterRenderer
	{
		public:
			EngineRasterRenderer(EngineWindow& window, EngineDevice& device);
			~EngineRasterRenderer();

			EngineRasterRenderer(const EngineRasterRenderer&) = delete;
			EngineRasterRenderer& operator = (const EngineRasterRenderer&) = delete;

			std::shared_ptr<EngineSwapChain> getSwapChain() const { return this->swapChain; }
			bool isFrameInProgress() const { return this->isFrameStarted; }
			
			std::shared_ptr<EngineDescriptorPool> getDescriptorPool() const { return this->descriptorPool; }
			std::shared_ptr<EngineDescriptorSetLayout> getGlobalDescSetLayout() const { return this->globalDescSetLayout; }
			std::shared_ptr<VkDescriptorSet> getGlobalDescriptorSets(int index) const { return this->globalDescriptorSets[index]; }

			VkCommandBuffer getCommandBuffer() const { 
				assert(this->isFrameStarted && "cannot get command buffer when frame is not in progress");
				return this->commandBuffers[this->currentFrameIndex]->getCommandBuffer();
			}

			int getFrameIndex() {
				assert(this->isFrameStarted && "cannot get frame index when frame is not in progress");
				return this->currentFrameIndex;
			}

			int getImageIndex() {
				assert(this->isFrameStarted && "cannot get frame index when frame is not in progress");
				return this->currentImageIndex;
			}

			void writeUniformBuffer(int frameIndex, void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
			void writeLightBuffer(int frameIndex, void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

			std::shared_ptr<EngineCommandBuffer> beginCommand();
			void endCommand(std::shared_ptr<EngineCommandBuffer>);

			void submitCommands(std::vector<std::shared_ptr<EngineCommandBuffer>> commandBuffer);
			void submitCommand(std::shared_ptr<EngineCommandBuffer> commandBuffer);

			bool acquireFrame();
			bool presentFrame();

		private:
			void recreateSwapChain();
			void createGlobalBuffers(unsigned long sizeUBO, unsigned long sizeLightBuffer);
			void createGlobalUboDescriptor();
			void createSyncObjects(uint32_t imageCount);

			EngineWindow& appWindow;
			EngineDevice& appDevice;

			std::shared_ptr<EngineSwapChain> swapChain;
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

			uint32_t currentImageIndex = 0;
			int currentFrameIndex = 0;
			bool isFrameStarted = false;
	};
}