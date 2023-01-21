#include "rasterization_renderer.hpp"
#include "../globalUbo.hpp"

#include <stdexcept>
#include <array>
#include <string>

namespace nugiEngine {
	EngineRasterRenderer::EngineRasterRenderer(EngineWindow& window, EngineDevice& device) : appDevice{device}, appWindow{window} {
		this->recreateSwapChain();
		this->createSyncObjects(static_cast<uint32_t>(this->swapChain->imageCount()));

		this->commandBuffers = EngineCommandBuffer::createCommandBuffers(device, EngineSwapChain::MAX_FRAMES_IN_FLIGHT);

		this->createGlobalBuffers(sizeof(GlobalUBO), sizeof(GlobalLight));
		this->createGlobalUboDescriptor();
	}

	EngineRasterRenderer::~EngineRasterRenderer() {
		// cleanup synchronization objects
    for (size_t i = 0; i < EngineSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroySemaphore(this->appDevice.getLogicalDevice(), this->renderFinishedSemaphores[i], nullptr);
      vkDestroySemaphore(this->appDevice.getLogicalDevice(), this->imageAvailableSemaphores[i], nullptr);
      vkDestroyFence(this->appDevice.getLogicalDevice(), this->inFlightFences[i], nullptr);
    }
	}

	void EngineRasterRenderer::recreateSwapChain() {
		auto extent = this->appWindow.getExtent();
		while(extent.width == 0 || extent.height == 0) {
			extent = this->appWindow.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(this->appDevice.getLogicalDevice());

		if (this->swapChain == nullptr) {
			this->swapChain = std::make_unique<EngineSwapChain>(this->appDevice, extent);
		} else {
			std::shared_ptr<EngineSwapChain> oldSwapChain = std::move(this->swapChain);
			this->swapChain = std::make_unique<EngineSwapChain>(this->appDevice, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormat(*this->swapChain.get())) {
				throw std::runtime_error("Swap chain image has changed");
			}
		}
	}

	void EngineRasterRenderer::createGlobalBuffers(unsigned long sizeUBO, unsigned long sizeLightBuffer) {
		this->globalUniformBuffers.resize(EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
		this->globalLightBuffers.resize(EngineSwapChain::MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < this->globalUniformBuffers.size(); i++) {
			this->globalUniformBuffers[i] = std::make_unique<EngineBuffer>(
				this->appDevice,
				sizeUBO,
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);

			this->globalLightBuffers[i] = std::make_unique<EngineBuffer>(
				this->appDevice,
				sizeLightBuffer,
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);

			this->globalUniformBuffers[i]->map();
			this->globalLightBuffers[i]->map();
		}
	}

	void EngineRasterRenderer::createGlobalUboDescriptor() {
		this->descriptorPool = 
			EngineDescriptorPool::Builder(this->appDevice)
				.setMaxSets(100 * EngineSwapChain::MAX_FRAMES_IN_FLIGHT)
				.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, EngineSwapChain::MAX_FRAMES_IN_FLIGHT)
				.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100)
				.build();

		this->globalDescSetLayout = 
			EngineDescriptorSetLayout::Builder(this->appDevice)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.build();

		this->globalDescriptorSets.resize(this->globalUniformBuffers.size());
		
		for (int i = 0; i < this->globalUniformBuffers.size(); i++) {
			this->globalDescriptorSets[i] = std::make_shared<VkDescriptorSet>();

			auto globalBufferInfo = this->globalUniformBuffers[i]->descriptorInfo();
			auto lightBufferInfo = this->globalLightBuffers[i]->descriptorInfo();

			EngineDescriptorWriter(*this->globalDescSetLayout, *this->descriptorPool)
				.writeBuffer(0, &globalBufferInfo)
				.writeBuffer(1, &lightBufferInfo)
				.build(this->globalDescriptorSets[i].get());
		}
	}

	void EngineRasterRenderer::createSyncObjects(uint32_t imageCount) {
    imageAvailableSemaphores.resize(EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(imageCount, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < EngineSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
      if (vkCreateSemaphore(this->appDevice.getLogicalDevice(), &semaphoreInfo, nullptr, &this->imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(this->appDevice.getLogicalDevice(), &semaphoreInfo, nullptr, &this->renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(this->appDevice.getLogicalDevice(), &fenceInfo, nullptr, &this->inFlightFences[i]) != VK_SUCCESS) 
      {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
      }
    }
  }

	void EngineRasterRenderer::writeUniformBuffer(int frameIndex, void* data, VkDeviceSize size, VkDeviceSize offset) {
		this->globalUniformBuffers[frameIndex]->writeToBuffer(data, size, offset);
		this->globalUniformBuffers[frameIndex]->flush(size, offset);
	}

	void EngineRasterRenderer::writeLightBuffer(int frameIndex, void* data, VkDeviceSize size, VkDeviceSize offset) {
		this->globalLightBuffers[frameIndex]->writeToBuffer(data, size, offset);
		this->globalLightBuffers[frameIndex]->flush(size, offset);
	}

	bool EngineRasterRenderer::acquireFrame() {
		assert(!this->isFrameStarted && "can't acquire frame while frame still in progress");

		auto result = this->swapChain->acquireNextImage(&this->currentImageIndex, &this->inFlightFences[this->currentFrameIndex], this->imageAvailableSemaphores[this->currentFrameIndex]);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			this->recreateSwapChain();
			return false;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image");
		}

		this->isFrameStarted = true;
		return true;
	}

	std::shared_ptr<EngineCommandBuffer> EngineRasterRenderer::beginCommand() {
		assert(this->isFrameStarted && "can't start command while frame still in progress");

		this->commandBuffers[this->currentFrameIndex]->beginReccuringCommand();
		return this->commandBuffers[this->currentFrameIndex];
	}

	void EngineRasterRenderer::endCommand(std::shared_ptr<EngineCommandBuffer> commandBuffer) {
		assert(this->isFrameStarted && "can't start command while frame still in progress");
		commandBuffer->endCommand();
	}

	void EngineRasterRenderer::submitCommands(std::vector<std::shared_ptr<EngineCommandBuffer>> commandBuffers) {
		assert(this->isFrameStarted && "can't submit command if frame is not in progress");

		if (this->imagesInFlight[this->currentImageIndex] != VK_NULL_HANDLE) {
      vkWaitForFences(this->appDevice.getLogicalDevice(), 1, &this->imagesInFlight[this->currentImageIndex], VK_TRUE, UINT64_MAX);
    }

    imagesInFlight[this->currentImageIndex] = this->inFlightFences[this->currentFrameIndex];
    vkResetFences(this->appDevice.getLogicalDevice(), 1, &this->inFlightFences[this->currentFrameIndex]);

    std::vector<VkSemaphore> waitSemaphores = {this->imageAvailableSemaphores[this->currentFrameIndex]};
		std::vector<VkSemaphore> signalSemaphores = {this->renderFinishedSemaphores[this->currentFrameIndex]};
    std::vector<VkPipelineStageFlags> waitStages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

		EngineCommandBuffer::submitCommands(commandBuffers, this->appDevice.getGraphicsQueue(), waitSemaphores, waitStages, signalSemaphores, this->inFlightFences[this->currentFrameIndex]);
	}

	void EngineRasterRenderer::submitCommand(std::shared_ptr<EngineCommandBuffer> commandBuffer) {
		assert(this->isFrameStarted && "can't submit command if frame is not in progress");

    if (this->imagesInFlight[this->currentImageIndex] != VK_NULL_HANDLE) {
      vkWaitForFences(this->appDevice.getLogicalDevice(), 1, &this->imagesInFlight[this->currentImageIndex], VK_TRUE, UINT64_MAX);
    }

    imagesInFlight[this->currentImageIndex] = this->inFlightFences[this->currentFrameIndex];
    vkResetFences(this->appDevice.getLogicalDevice(), 1, &this->inFlightFences[this->currentFrameIndex]);

    std::vector<VkSemaphore> waitSemaphores = {this->imageAvailableSemaphores[this->currentFrameIndex]};
		std::vector<VkSemaphore> signalSemaphores = {this->renderFinishedSemaphores[this->currentFrameIndex]};
    std::vector<VkPipelineStageFlags> waitStages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

		commandBuffer->submitCommand(this->appDevice.getGraphicsQueue(), waitSemaphores, waitStages, signalSemaphores, this->inFlightFences[this->currentFrameIndex]);
	}

	bool EngineRasterRenderer::presentFrame() {
		assert(this->isFrameStarted && "can't present frame if frame is not in progress");

		auto result = this->swapChain->presentRenders(&this->currentImageIndex, &this->renderFinishedSemaphores[this->currentFrameIndex]);

		this->currentFrameIndex = (this->currentFrameIndex + 1) % EngineSwapChain::MAX_FRAMES_IN_FLIGHT;
		this->isFrameStarted = false;

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->appWindow.wasResized()) {
			this->appWindow.resetResizedFlag();
			this->recreateSwapChain();

			return false;
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image");
		}

		return true;
	}
}