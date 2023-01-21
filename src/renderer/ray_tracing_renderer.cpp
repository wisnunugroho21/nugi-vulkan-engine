#include "ray_tracing_renderer.hpp"
#include "../ray_ubo.hpp"

#include <stdexcept>
#include <array>
#include <string>

namespace nugiEngine {
	EngineRayTraceRenderer::EngineRayTraceRenderer(EngineWindow& window, EngineDevice& device) : appDevice{device}, appWindow{window} {
		this->recreateSwapChain();
		this->createSyncObjects(static_cast<uint32_t>(this->swapChain->imageCount()));

		this->commandBuffers = EngineCommandBuffer::createCommandBuffers(device, EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
		this->createDescriptorPool();
	}

	EngineRayTraceRenderer::~EngineRayTraceRenderer() {
		// cleanup synchronization objects
		this->descriptorPool->resetPool();
		
    for (size_t i = 0; i < EngineSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroySemaphore(this->appDevice.getLogicalDevice(), this->renderFinishedSemaphores[i], nullptr);
      vkDestroySemaphore(this->appDevice.getLogicalDevice(), this->imageAvailableSemaphores[i], nullptr);
      vkDestroyFence(this->appDevice.getLogicalDevice(), this->inFlightFences[i], nullptr);
    }
	}

	void EngineRayTraceRenderer::recreateSwapChain() {
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

	void EngineRayTraceRenderer::createDescriptorPool() {
		uint32_t nSample = 8;
		uint32_t imageCount = static_cast<uint32_t>(this->swapChain->getswapChainImages().size());

		this->descriptorPool = 
			EngineDescriptorPool::Builder(this->appDevice)
				.setMaxSets(imageCount * nSample + imageCount)
				.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, imageCount * nSample + imageCount * 2)
				.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 * imageCount * nSample)
				.build();
	}

	void EngineRayTraceRenderer::createSyncObjects(uint32_t imageCount) {
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

	bool EngineRayTraceRenderer::acquireFrame() {
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

	std::shared_ptr<EngineCommandBuffer> EngineRayTraceRenderer::beginCommand() {
		assert(this->isFrameStarted && "can't start command while frame still in progress");

		this->commandBuffers[this->currentFrameIndex]->beginReccuringCommand();
		return this->commandBuffers[this->currentFrameIndex];
	}

	void EngineRayTraceRenderer::endCommand(std::shared_ptr<EngineCommandBuffer> commandBuffer) {
		assert(this->isFrameStarted && "can't start command while frame still in progress");
		commandBuffer->endCommand();
	}

	void EngineRayTraceRenderer::submitCommands(std::vector<std::shared_ptr<EngineCommandBuffer>> commandBuffers) {
		assert(this->isFrameStarted && "can't submit command if frame is not in progress");

		if (this->imagesInFlight[this->currentImageIndex] != VK_NULL_HANDLE) {
      vkWaitForFences(this->appDevice.getLogicalDevice(), 1, &this->imagesInFlight[this->currentImageIndex], VK_TRUE, UINT64_MAX);
    }

    imagesInFlight[this->currentImageIndex] = this->inFlightFences[this->currentFrameIndex];
    vkResetFences(this->appDevice.getLogicalDevice(), 1, &this->inFlightFences[this->currentFrameIndex]);

    std::vector<VkSemaphore> waitSemaphores = {this->imageAvailableSemaphores[this->currentFrameIndex]};
		std::vector<VkSemaphore> signalSemaphores = {this->renderFinishedSemaphores[this->currentFrameIndex]};
    std::vector<VkPipelineStageFlags> waitStages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

		EngineCommandBuffer::submitCommands(commandBuffers, this->appDevice.getComputeQueue(), waitSemaphores, waitStages, signalSemaphores, this->inFlightFences[this->currentFrameIndex]);
	}

	void EngineRayTraceRenderer::submitCommand(std::shared_ptr<EngineCommandBuffer> commandBuffer) {
		assert(this->isFrameStarted && "can't submit command if frame is not in progress");

    if (this->imagesInFlight[this->currentImageIndex] != VK_NULL_HANDLE) {
      vkWaitForFences(this->appDevice.getLogicalDevice(), 1, &this->imagesInFlight[this->currentImageIndex], VK_TRUE, UINT64_MAX);
    }

    imagesInFlight[this->currentImageIndex] = this->inFlightFences[this->currentFrameIndex];
    vkResetFences(this->appDevice.getLogicalDevice(), 1, &this->inFlightFences[this->currentFrameIndex]);

    std::vector<VkSemaphore> waitSemaphores = {this->imageAvailableSemaphores[this->currentFrameIndex]};
		std::vector<VkSemaphore> signalSemaphores = {this->renderFinishedSemaphores[this->currentFrameIndex]};
    std::vector<VkPipelineStageFlags> waitStages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

		commandBuffer->submitCommand(this->appDevice.getComputeQueue(), waitSemaphores, waitStages, signalSemaphores, this->inFlightFences[this->currentFrameIndex]);
	}

	bool EngineRayTraceRenderer::presentFrame() {
		assert(this->isFrameStarted && "can't present frame if frame is not in progress");

		auto result = this->swapChain->presentRenders(&this->currentImageIndex, &this->renderFinishedSemaphores[this->currentFrameIndex]);

		this->currentFrameIndex = (this->currentFrameIndex + 1) % EngineSwapChain::MAX_FRAMES_IN_FLIGHT;
		this->isFrameStarted = false;

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->appWindow.wasResized()) {
			this->appWindow.resetResizedFlag();
			this->recreateSwapChain();
			this->descriptorPool->resetPool();

			this->randomSeed = 0;
			return false;
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image");
		}

		this->randomSeed++;
		return true;
	}
}