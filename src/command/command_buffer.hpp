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

      void createCommandBuffers(uint32_t size);
      void freeCommandBuffers();

      void beginSingleTimeCommands(int index = -1);
      void beginReccuringCommands(int index = -1);
      void endCommands(int index = -1);

      VkCommandBuffer getBuffer(int index) { return this->commandBuffers[index]; }

    private:
      EngineDevice& device;
      std::vector<VkCommandBuffer> commandBuffers;
  };
  
} // namespace nugiEngine

