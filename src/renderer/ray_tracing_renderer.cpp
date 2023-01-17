#include "ray_tracing_renderer.hpp"
#include "../globalUbo.hpp"

#include <stdexcept>
#include <array>
#include <string>

namespace nugiEngine {
	EngineRayTraceRenderer::EngineRayTraceRenderer(EngineWindow& window, EngineDevice& device) : appDevice{device}, appWindow{window} {
		this->recreateSwapChain();
		this->createSyncObjects(this->swapChain->imageCount());

		this->commandBuffers = EngineCommandBuffer::createCommandBuffers(device, EngineSwapChain::MAX_FRAMES_IN_FLIGHT);

		this->createStorageImage();
		this->createGlobalDescriptor();
	}

	EngineRayTraceRenderer::~EngineRayTraceRenderer() {
		// cleanup synchronization objects
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

	void EngineRayTraceRenderer::createStorageImage() {
		this->storageImage = std::make_unique<EngineImage>(this->appDevice, this->swapChain->getSwapChainExtent().width, this->swapChain->getSwapChainExtent().height, 
			1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_B8G8R8A8_UNORM, 
      VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, 
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

		this->storageImage->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	}

	void EngineRayTraceRenderer::createGlobalDescriptor() {
		this->descriptorPool = 
			EngineDescriptorPool::Builder(this->appDevice)
				.setMaxSets(1)
				.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
				.build();

		this->globalDescSetLayout = 
			EngineDescriptorSetLayout::Builder(this->appDevice)
				.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
				.build();

		VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = this->storageImage->getImageView();

		EngineDescriptorWriter(*this->globalDescSetLayout, *this->descriptorPool)
			.writeImage(0, &imageInfo)
			.build(this->globalDescriptorSets.get());
	}

	void EngineRayTraceRenderer::createSyncObjects(int imageCount) {
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

			return false;
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image");
		}

		return true;
	}

	bool EngineRayTraceRenderer::copyFrameToSwapChain(std::shared_ptr<EngineCommandBuffer> commandBuffer) {
		this->storageImage->transitionImageLayout(VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, commandBuffer);

		this->swapChain->getswapChainImages()[this->getImageIndex()]->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, commandBuffer);

		this->swapChain->getswapChainImages()[this->getImageIndex()]->copyImageFromOther(this->storageImage, 
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandBuffer);

		this->storageImage->transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, 
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, commandBuffer);

		this->swapChain->getswapChainImages()[this->getImageIndex()]->transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, commandBuffer);
	}
}