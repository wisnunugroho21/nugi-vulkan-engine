#pragma once

#include "bottom_level_acceleration_structure.hpp"
#include "../buffer/buffer.hpp"
#include "../device/device.hpp"
#include "../command/command_buffer.hpp"

namespace nugiEngine {
  class EngineTopLevelAccelerationStructure
  {
    public:
      EngineTopLevelAccelerationStructure(EngineDevice& appDevice, std::vector<EngineBottomLevelAccelerationStructure> bottomLevelAccelStructs);

    private:
      EngineDevice& appDevice;

      VkAccelerationStructureKHR accelStruct{};
      VkDeviceAddress address;
      std::shared_ptr<EngineBuffer> buffer;

      void createTopLevelAccelerationStructure(std::vector<EngineBottomLevelAccelerationStructure> bottomLevelAccelStructs);
  };
  
} // namespace nugiEngine

