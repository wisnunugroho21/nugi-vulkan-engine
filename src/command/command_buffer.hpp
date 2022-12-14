#pragma once

#include "../device/device.hpp"

#include <vector>
#include <memory>

namespace nugiEngine
{
  class EngineCommandBuffer {
    public:
      EngineCommandBuffer(EngineDevice& device, VkCommandBuffer commandBuffer);
      EngineCommandBuffer(EngineDevice& device);

      ~EngineCommandBuffer();

      EngineCommandBuffer(const EngineCommandBuffer&) = delete;
      EngineCommandBuffer& operator=(const EngineCommandBuffer&) = delete;

      static std::vector<std::shared_ptr<EngineCommandBuffer>> createCommandBuffers(EngineDevice &appDevice, uint32_t size);

      void beginSingleTimeCommand();
      void beginReccuringCommand();
      void endCommand();
      void submitCommand(VkQueue queue, std::vector<VkSemaphore> waitSemaphores = {}, 
        std::vector<VkPipelineStageFlags> waitStages = {}, std::vector<VkSemaphore> signalSemaphores = {}, 
        VkFence fence = VK_NULL_HANDLE);

      static void submitCommands(std::vector<std::shared_ptr<EngineCommandBuffer>> commandBuffers, VkQueue queue, std::vector<VkSemaphore> waitSemaphores = {}, 
        std::vector<VkPipelineStageFlags> waitStages = {}, std::vector<VkSemaphore> signalSemaphores = {}, 
        VkFence fence = VK_NULL_HANDLE);

      VkCommandBuffer getCommandBuffer() const { return this->commandBuffer; }

    private:
      EngineDevice& appDevice;
      VkCommandBuffer commandBuffer;
  };
  
} // namespace nugiEngine

