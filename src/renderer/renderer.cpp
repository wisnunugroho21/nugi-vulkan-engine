#include "renderer.hpp"
#include "../globalUbo.hpp"

#include <stdexcept>
#include <array>
#include <string>

namespace nugiEngine {
	EngineRenderer::EngineRenderer(EngineWindow& window, EngineDevice& device) : appDevice{device}, appWindow{window} {
		this->recreateSwapChain();
		this->commandBuffers = std::make_unique<EngineCommandBuffer>(device, EngineSwapChain::MAX_FRAMES_IN_FLIGHT);

		this->createGlobalBuffers(sizeof(GlobalUBO), sizeof(GlobalLight));
		this->createGlobalUboDescriptor();
	}

	EngineRenderer::~EngineRenderer() {}

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

	void EngineRenderer::createGlobalBuffers(unsigned long sizeUBO, unsigned long sizeLightBuffer) {
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

	void EngineRenderer::createGlobalUboDescriptor() {
		this->descriptorPool = 
			EngineDescriptorPool::Builder(this->appDevice)
				.setMaxSets(100 * EngineSwapChain::MAX_FRAMES_IN_FLIGHT)
				.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, EngineSwapChain::MAX_FRAMES_IN_FLIGHT)
				.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100)
				.build();

		this->globalDescSetLayout = 
			EngineDescriptorSetLayout::Builder(this->appDevice)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
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

	void EngineRenderer::writeUniformBuffer(int frameIndex, void* data, VkDeviceSize size, VkDeviceSize offset) {
		this->globalUniformBuffers[frameIndex]->writeToBuffer(data, size, offset);
		this->globalUniformBuffers[frameIndex]->flush(size, offset);
	}

	void EngineRenderer::writeLightBuffer(int frameIndex, void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
		this->globalLightBuffers[frameIndex]->writeToBuffer(data, size, offset);
		this->globalLightBuffers[frameIndex]->flush(size, offset);
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

		this->commandBuffers->beginReccuringCommands(this->currentFrameIndex);
		return this->commandBuffers->getBuffer(this->currentFrameIndex);
	}

	void EngineRenderer::endFrame(VkCommandBuffer commandBuffer) {
		assert(this->isFrameStarted && "can't call endframe if frame is not in progress");

		EngineCommandBuffer::endCommands(commandBuffer);

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