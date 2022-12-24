#pragma once

#include "../buffer/buffer.hpp"
#include "../device/device.hpp"

namespace nugiEngine
{
  class EngineShaderBindingTable
  {
    private:
      EngineDevice& appDevice;

      std::unique_ptr<EngineBuffer> raygenSBTBuffer;
      std::unique_ptr<EngineBuffer> missgenSBTBuffer;
      std::unique_ptr<EngineBuffer> hitgenSBTBuffer;

      void createShaderBindingTables();

      static uint32_t aligned_size(uint32_t value, uint32_t alignment) { return (value + alignment - 1) & ~(alignment - 1); }
  };
} // namespace nugiEngine
