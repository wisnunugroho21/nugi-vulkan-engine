#pragma once

#include "../device/device.hpp"

#include <vector>

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

      void beginSingleTimeCommands();
      void beginReccuringCommands();
      void endCommands();
      void submitCommands(VkQueue queue, std::vector<VkSemaphore> *waitSemaphores = nullptr, 
        std::vector<VkPipelineStageFlags> *waitStages = nullptr, std::vector<VkSemaphore> *signalSemaphores = nullptr, 
        VkFence fence = VK_NULL_HANDLE);

      VkCommandBuffer getCommandBuffer() const { return this->commandBuffer; }

    private:
      EngineDevice& appDevice;
      VkCommandBuffer commandBuffer;
  };
  
} // namespace nugiEngine

