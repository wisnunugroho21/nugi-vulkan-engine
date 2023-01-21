#include "command_buffer.hpp"

#include <iostream>

namespace nugiEngine {
	EngineCommandBuffer::~EngineCommandBuffer() {
		vkFreeCommandBuffers(
      this->appDevice.getLogicalDevice(), 
      this->appDevice.getCommandPool(), 
      1, 
      &this->commandBuffer
    );
	}

  std::vector<std::shared_ptr<EngineCommandBuffer>> EngineCommandBuffer::createCommandBuffers(EngineDevice &appDevice, uint32_t size) {
		std::vector<VkCommandBuffer> commandBuffers{size};
		std::vector<std::shared_ptr<EngineCommandBuffer>> appCommandBuffers;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = appDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(appDevice.getLogicalDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffer");
		}

		for (size_t i = 0; i < size; i++) {
			appCommandBuffers.push_back(
				std::make_shared<EngineCommandBuffer>(appDevice, commandBuffers[i])
			);
		}

		return appCommandBuffers;
	}

	EngineCommandBuffer::EngineCommandBuffer(EngineDevice& device, VkCommandBuffer commandBuffer) 
		: appDevice{device}, commandBuffer {commandBuffer} 
	{

	}

	EngineCommandBuffer::EngineCommandBuffer(EngineDevice& device) : appDevice{device} {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = this->appDevice.getCommandPool();
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(appDevice.getLogicalDevice(), &allocInfo, &this->commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffer");
		}
	}

	void EngineCommandBuffer::beginSingleTimeCommand() {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(this->commandBuffer, &beginInfo) != VK_SUCCESS) {
			std::cerr << "Failed to start recording buffer" << '\n';
		}
	}

	void EngineCommandBuffer::beginReccuringCommand() {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(this->commandBuffer, &beginInfo) != VK_SUCCESS) {
			std::cerr << "Failed to start recording command buffer" << '\n';
		}
	}

	void EngineCommandBuffer::endCommand() {
		if (vkEndCommandBuffer(this->commandBuffer) != VK_SUCCESS) {
			std::cerr << "Failed to end recording command buffer" << '\n';
		}
	}

	void EngineCommandBuffer::submitCommand(VkQueue queue, std::vector<VkSemaphore> waitSemaphores, 
    std::vector<VkPipelineStageFlags> waitStages, std::vector<VkSemaphore> signalSemaphores, VkFence fence) 
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &this->commandBuffer;

		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
		if (waitSemaphores.size() == 0) {
  		submitInfo.pWaitSemaphores = nullptr;
		} else {
			submitInfo.pWaitSemaphores = waitSemaphores.data();
		}

		if (waitStages.size() > 0) {
			submitInfo.pWaitDstStageMask = waitStages.data();
		}

		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
		if (signalSemaphores.size() == 0) {
  		submitInfo.pSignalSemaphores = nullptr;
		} else {
			submitInfo.pSignalSemaphores = signalSemaphores.data();
		}

		if (vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS) {
			std::cerr << "Failed to submitting command buffer" << '\n';
		}

		if (vkQueueWaitIdle(queue) != VK_SUCCESS) {
			std::cerr << "Failed to waiting queue" << '\n';
		}
	}

	void EngineCommandBuffer::submitCommands(std::vector<std::shared_ptr<EngineCommandBuffer>> commandBuffers, VkQueue queue, std::vector<VkSemaphore> waitSemaphores, 
		std::vector<VkPipelineStageFlags> waitStages, std::vector<VkSemaphore> signalSemaphores, VkFence fence) 
	{
		std::vector<VkCommandBuffer> buffers{};
		for (auto& commandBuffer : commandBuffers) {
			buffers.push_back(commandBuffer->getCommandBuffer());
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = static_cast<uint32_t>(buffers.size());
		submitInfo.pCommandBuffers = buffers.data();

		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
		if (waitSemaphores.size() == 0) {
  		submitInfo.pWaitSemaphores = nullptr;
		} else {
			submitInfo.pWaitSemaphores = waitSemaphores.data();
		}

		if (waitStages.size() > 0) {
			submitInfo.pWaitDstStageMask = waitStages.data();
		}

		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
		if (signalSemaphores.size() == 0) {
  		submitInfo.pSignalSemaphores = nullptr;
		} else {
			submitInfo.pSignalSemaphores = signalSemaphores.data();
		}

		if (vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS) {
			std::cerr << "Failed to submitting command buffer" << '\n';
		}

		if (vkQueueWaitIdle(queue) != VK_SUCCESS) {
			std::cerr << "Failed to waiting queue" << '\n';
		}
		
	}
} // namespace nugiEngine
