#include "renderer.hpp"

#include <stdexcept>
#include <array>
#include <string>

namespace nugiEngine {
	EngineRenderer::EngineRenderer(EngineWindow& window, EngineDevice& device) : appDevice{device}, appWindow{window} {
		this->recreateSwapChain();
		this->createCommandBuffers();
	}

	EngineRenderer::~EngineRenderer() {
		this->freeCommandBuffers();
	}

	void EngineRenderer::recreateSwapChain() {
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
				throw std::runtime_error("Swap chain image or depth has changed");
			}
		}

	}

	void EngineRenderer::createCommandBuffers() {
		this->commandBuffers.resize(EngineSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = this->appDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(this->commandBuffers.size());

		if (vkAllocateCommandBuffers(this->appDevice.getLogicalDevice(), &allocInfo, this->commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffer");
		}
	}

	void EngineRenderer::freeCommandBuffers() {
		vkFreeCommandBuffers(this->appDevice.getLogicalDevice(), this->appDevice.getCommandPool(), static_cast<uint32_t>(this->commandBuffers.size()), this->commandBuffers.data());
		this->commandBuffers.clear();
	}

	VkCommandBuffer EngineRenderer::beginFrame() {
		assert(!this->isFrameStarted && "can't call beginframe while frame still in progress");

		auto result = this->swapChain->acquireNextImage(&this->currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			this->recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image");
		}

		this->isFrameStarted = true;
		auto commandBuffer = this->getCommandBuffer();

		VkCommandBufferBeginInfo commandBeginInfo{};
		commandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &commandBeginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer " + std::to_string(this->currentImageIndex));
		}

		return commandBuffer;
	}

	void EngineRenderer::endFrame(VkCommandBuffer commandBuffer) {
		assert(this->isFrameStarted && "can't call endframe if frame is not in progress");

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer");
		}

		auto result = this->swapChain->executeAndPresentRenders(&commandBuffer, &this->currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->appWindow.wasResized()) {
			this->appWindow.resetResizedFlag();
			this->recreateSwapChain();
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image");
		}

		this->isFrameStarted = false;
		this->currentFrameIndex = (this->currentFrameIndex + 1) % EngineSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void EngineRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(this->isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
		assert(commandBuffer == this->getCommandBuffer() && "Can't begin render pass on command buffer from different frame");

		VkRenderPassBeginInfo renderBeginInfo{};
		renderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderBeginInfo.renderPass = this->swapChain->getRenderPass();
		renderBeginInfo.framebuffer = this->swapChain->getFrameBuffer(this->currentImageIndex);

		renderBeginInfo.renderArea.offset = {0, 0};
		renderBeginInfo.renderArea.extent = this->swapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
		clearValues[1].depthStencil = {1.0f, 0};
		renderBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<uint32_t>(this->swapChain->getSwapChainExtent().width);
		viewport.height = static_cast<uint32_t>(this->swapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{{0, 0}, this->swapChain->getSwapChainExtent()};
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void EngineRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(this->isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
		assert(commandBuffer == this->getCommandBuffer() && "Can't end render pass on command buffer from different frame");

		vkCmdEndRenderPass(commandBuffer);
	}
}