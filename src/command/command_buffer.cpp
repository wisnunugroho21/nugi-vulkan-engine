#include "command_buffer.hpp"

#include <iostream>

namespace nugiEngine {
	EngineCommandBuffer::EngineCommandBuffer(EngineDevice& device, uint32_t size) : device(device) {
		this->createCommandBuffers(size);
	}

	EngineCommandBuffer::~EngineCommandBuffer() {
		this->freeCommandBuffers();
	}

  void EngineCommandBuffer::createCommandBuffers(uint32_t size) {
		this->commandBuffers.resize(size);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = this->device.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(this->commandBuffers.size());

		if (vkAllocateCommandBuffers(this->device.getLogicalDevice(), &allocInfo, this->commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffer");
		}
	}

  void EngineCommandBuffer::freeCommandBuffers() {
		vkFreeCommandBuffers(
      this->device.getLogicalDevice(), 
      this->device.getCommandPool(), 
      static_cast<uint32_t>(this->commandBuffers.size()), 
      this->commandBuffers.data()
    );
    
		this->commandBuffers.clear();
	}

	void EngineCommandBuffer::beginSingleTimeCommands(int index = -1) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (index == -1) {
			index = this->commandBuffers.size() - 1;
		}

		if (vkBeginCommandBuffer(this->commandBuffers[index], &beginInfo) != VK_SUCCESS) {
			std::cerr << "Failed to start recording buffer" << '\n';
		}
	}

	void EngineCommandBuffer::beginReccuringCommands(int index = -1) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (index == -1) {
			index = this->commandBuffers.size() - 1;
		}

		if (vkBeginCommandBuffer(this->commandBuffers[index], &beginInfo) != VK_SUCCESS) {
			std::cerr << "Failed to start recording buffer" << '\n';
		}
	}

	void EngineCommandBuffer::endCommands(int index = -1) {
		if (index == -1) {
			index = this->commandBuffers.size() - 1;
		}

		if (vkEndCommandBuffer(this->commandBuffers[index]) != VK_SUCCESS) {
			std::cerr << "Failed to start recording buffer" << '\n';
		}
	}
} // namespace nugiEngine
