#pragma once

#include "../device/device.hpp"

#include <vector>

namespace nugiEngine
{
  class EngineCommandBuffer {
    public:
      EngineCommandBuffer(EngineDevice& device, uint32_t size);
      ~EngineCommandBuffer();

      EngineCommandBuffer(const EngineCommandBuffer&) = delete;
      EngineCommandBuffer& operator=(const EngineCommandBuffer&) = delete;

      void beginSingleTimeCommands(int32_t index = -1);
      void beginReccuringCommands(int32_t index = -1);
      void endCommands(int32_t index = -1);
      void submitCommands(VkQueue queue, int32_t index = -1, std::vector<VkSemaphore> *waitSemaphores = nullptr, 
        std::vector<VkPipelineStageFlags> *waitStages = nullptr, std::vector<VkSemaphore> *signalSemaphores = nullptr, 
        VkFence fence = VK_NULL_HANDLE);

      VkCommandBuffer getBuffer(int32_t index = -1);

    private:
      EngineDevice& device;
      std::vector<VkCommandBuffer> commandBuffers;

      void createCommandBuffers(uint32_t size);
      void freeCommandBuffers();
  };
  
} // namespace nugiEngine

