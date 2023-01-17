#include "ray_tracing_renderer.hpp"
#include "../ray_ubo.hpp"

#include <stdexcept>
#include <array>
#include <string>

namespace nugiEngine {
	EngineRayTraceRenderer::EngineRayTraceRenderer(EngineWindow& window, EngineDevice& device) : appDevice{device}, appWindow{window} {
		this->recreateSwapChain();
		this->createSyncObjects(this->swapChain->imageCount());

		this->commandBuffers = EngineCommandBuffer::createCommandBuffers(device, EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
		this->createGlobalUniformBuffer(sizeof(RayTraceUbo));
		
		this->createGlobalDescriptor();
		this->recreateDescriptor();
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

	void EngineRayTraceRenderer::createGlobalDescriptor() {
		uint32_t imageCount = static_cast<uint32_t>(this->swapChain->getswapChainImages().size());

		this->descriptorPool = 
			EngineDescriptorPool::Builder(this->appDevice)
				.setMaxSets(imageCount)
				.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, imageCount)
				.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, imageCount)
				.build();

		this->globalDescSetLayout = 
			EngineDescriptorSetLayout::Builder(this->appDevice)
				.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
				.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
				.build();
	}

	void EngineRayTraceRenderer::recreateDescriptor() {
		this->descriptorPool->resetPool();
		this->globalDescriptorSets.clear();

		uint32_t imageCount = static_cast<uint32_t>(this->swapChain->getswapChainImages().size());

		for (uint32_t i = 0; i < imageCount; i++) {
			auto descSet = std::make_shared<VkDescriptorSet>();

			auto swapChainImage = this->swapChain->getswapChainImages()[i];
			auto uniformBuffer = this->globalUniformBuffers[i];

			auto imageInfo = swapChainImage->getDescriptorInfo(VK_IMAGE_LAYOUT_GENERAL);
			auto bufferInfo = uniformBuffer->descriptorInfo();

			EngineDescriptorWriter(*this->globalDescSetLayout, *this->descriptorPool)
				.writeImage(0, &imageInfo) 
				.writeBuffer(1, &bufferInfo)
				.build(descSet.get());

			this->globalDescriptorSets.emplace_back(descSet);
		}
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

	void EngineRayTraceRenderer::createGlobalUniformBuffer(unsigned long sizeUBO) {
		uint32_t imageCount = static_cast<uint32_t>(this->swapChain->getswapChainImages().size());

		for (uint32_t i = 0; i < imageCount; i++) {
			auto uniformBuffer = std::make_shared<EngineBuffer>(
				this->appDevice,
				sizeUBO,
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);

			uniformBuffer->map();
			this->globalUniformBuffers.emplace_back(uniformBuffer);
		}
	}

	void EngineRayTraceRenderer::writeGlobalData(int imageIndex) {
		RayTraceUbo ubo;

		auto viewport_height = 2.0f;
    auto viewport_width = 1.0f * viewport_height;
    auto focal_length = 1.0f;

		ubo.origin = glm::vec3(0.0f, 0.0f, 0.0f);
    ubo.horizontal = glm::vec3(viewport_width, 0.0f, 0.0f);
    ubo.vertical = glm::vec3(0.0f, viewport_height, 0.0f);
    ubo.lowerLeftCorner = ubo.origin - ubo.horizontal / 2.0f - ubo.vertical / 2.0f - glm::vec3(0.0f, 0.0f, focal_length);

		this->globalUniformBuffers[imageIndex]->writeToBuffer(&ubo);
		this->globalUniformBuffers[imageIndex]->flush();
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

	bool EngineRayTraceRenderer::prepareFrame(std::shared_ptr<EngineCommandBuffer> commandBuffer) {
		auto imageIndex = this->getImageIndex();
		auto swapChainImage = this->swapChain->getswapChainImages()[imageIndex];

		swapChainImage->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, commandBuffer);

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

	bool EngineRayTraceRenderer::finishFrame(std::shared_ptr<EngineCommandBuffer> commandBuffer) {
		auto imageIndex = this->getImageIndex();
		auto swapChainImage = this->swapChain->getswapChainImages()[imageIndex];

		swapChainImage->transitionImageLayout(VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, commandBuffer);

    return true;
	}

	bool EngineRayTraceRenderer::presentFrame() {
		assert(this->isFrameStarted && "can't present frame if frame is not in progress");

		auto result = this->swapChain->presentRenders(&this->currentImageIndex, &this->renderFinishedSemaphores[this->currentFrameIndex]);

		this->currentFrameIndex = (this->currentFrameIndex + 1) % EngineSwapChain::MAX_FRAMES_IN_FLIGHT;
		this->isFrameStarted = false;

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->appWindow.wasResized()) {
			this->appWindow.resetResizedFlag();
			this->recreateSwapChain();
			this->recreateDescriptor();

			return false;
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image");
		}

		return true;
	}
}