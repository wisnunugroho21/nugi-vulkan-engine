#pragma once

#include "../buffer/buffer.hpp"
#include "../model/model.hpp"
#include "../device/device.hpp"
#include "../command/command_buffer.hpp"
#include "../device/device_procedures.hpp"

#include <memory>

namespace nugiEngine {
  class EngineBottomLevelAccelerationStructure
  {
    public:
      EngineBottomLevelAccelerationStructure(EngineDevice& appDevice, EngineDeviceProcedures& deviceProcedure, std::shared_ptr<EngineModel> model);

      VkAccelerationStructureKHR getAccelStruct() const { return this->accelStruct; }
      VkDeviceAddress getAddress() const { return this->address; }
      
    private:
      EngineDevice& appDevice;
      EngineDeviceProcedures& deviceProcedure;

      VkAccelerationStructureKHR accelStruct{};
      VkDeviceAddress address;
      std::shared_ptr<EngineBuffer> buffer;

      void createBottomLevelAccelerationStructure(std::shared_ptr<EngineModel> model);
  };
}
