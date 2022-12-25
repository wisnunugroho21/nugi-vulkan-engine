#pragma once

#include "bottom_level_acceleration_structure.hpp"
#include "../buffer/buffer.hpp"
#include "../device/device.hpp"
#include "../command/command_buffer.hpp"

namespace nugiEngine {
  class EngineTopLevelAccelerationStructure
  {
    public:
      EngineTopLevelAccelerationStructure(EngineDevice& appDevice, EngineDeviceProcedures& deviceProcedure, EngineBottomLevelAccelerationStructure bottomLevelAccelStruct);

      VkAccelerationStructureKHR getAccelStruct() const { return this->accelStruct; }
      VkDeviceAddress getAddress() const { return this->address; }

    private:
      EngineDevice& appDevice;
      EngineDeviceProcedures& deviceProcedure;

      VkAccelerationStructureKHR accelStruct{};
      VkDeviceAddress address;
      std::shared_ptr<EngineBuffer> buffer;

      void createTopLevelAccelerationStructure(EngineBottomLevelAccelerationStructure bottomLevelAccelStruct);
  };
  
} // namespace nugiEngine

