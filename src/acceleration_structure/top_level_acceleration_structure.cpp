#include "top_level_acceleration_structure.hpp"

namespace nugiEngine {
	EngineTopLevelAccelerationStructure::EngineTopLevelAccelerationStructure(
		EngineDevice& appDevice, std::vector<EngineBottomLevelAccelerationStructure> bottomLevelAccelStructs)
		: appDevice{appDevice}
	{
		this->createTopLevelAccelerationStructure(bottomLevelAccelStructs);
	}

  void EngineTopLevelAccelerationStructure::createTopLevelAccelerationStructure(std::vector<EngineBottomLevelAccelerationStructure> bottomLevelAccelStructs) {
		std::vector<VkAccelerationStructureGeometryKHR> geometries;

		for (auto &&bottomStruc : bottomLevelAccelStructs) {
			VkAccelerationStructureInstanceKHR instance{};
			instance.transform = {};
			instance.instanceCustomIndex = 0;
			instance.mask = 0xFF;
			instance.instanceShaderBindingTableRecordOffset = 0;
			instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
			instance.accelerationStructureReference = bottomStruc.getAddress();

			EngineBuffer instancesBuffer{
				this->appDevice,
				sizeof(VkAccelerationStructureInstanceKHR),
				static_cast<uint32_t>(1),
				VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			};

			VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
			instanceDataDeviceAddress.deviceAddress = instancesBuffer.getDeviceAddress();

			VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
			accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
			accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
			accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
			accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
			accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
			accelerationStructureGeometry.geometry.instances.data = instanceDataDeviceAddress;

			geometries.emplace_back(accelerationStructureGeometry);
		}

    uint32_t primitiveCount = static_cast<uint32_t>(geometries.size());

    VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
		accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationStructureBuildGeometryInfo.geometryCount = static_cast<uint32_t>(geometries.size());
		accelerationStructureBuildGeometryInfo.pGeometries = geometries.data();

    VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
		accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		vkGetAccelerationStructureBuildSizesKHR(
			this->appDevice.getLogicalDevice(), 
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&accelerationStructureBuildGeometryInfo,
			&primitiveCount,
			&accelerationStructureBuildSizesInfo
    );

    this->buffer = std::make_shared<EngineBuffer>(
      this->appDevice,
			accelerationStructureBuildSizesInfo.accelerationStructureSize,
			static_cast<uint32_t>(1),
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
		accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		accelerationStructureCreateInfo.buffer = this->buffer->getBuffer();
		accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
		accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		vkCreateAccelerationStructureKHR(this->appDevice.getLogicalDevice(), &accelerationStructureCreateInfo, nullptr, &this->accelStruct);

    // Create a small scratch buffer used during build of the top level acceleration structure
    EngineBuffer stratchBuffer{
			this->appDevice,
			accelerationStructureBuildSizesInfo.buildScratchSize,
			static_cast<uint32_t>(1),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		};

		VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
		accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		accelerationBuildGeometryInfo.dstAccelerationStructure = this->accelStruct;
		accelerationBuildGeometryInfo.geometryCount = static_cast<uint32_t>(geometries.size());
		accelerationBuildGeometryInfo.pGeometries = geometries.data();
		accelerationBuildGeometryInfo.scratchData.deviceAddress = stratchBuffer.getDeviceAddress();

		VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
		accelerationStructureBuildRangeInfo.primitiveCount = primitiveCount;
		accelerationStructureBuildRangeInfo.primitiveOffset = 0;
		accelerationStructureBuildRangeInfo.firstVertex = 0;
		accelerationStructureBuildRangeInfo.transformOffset = 0;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

    EngineCommandBuffer commandBuffer{this->appDevice};
		commandBuffer.beginSingleTimeCommand();

		vkCmdBuildAccelerationStructuresKHR(
			commandBuffer.getCommandBuffer(),
			1,
			&accelerationBuildGeometryInfo,
			accelerationBuildStructureRangeInfos.data()
		);

		commandBuffer.endCommand();
		commandBuffer.submitCommand(this->appDevice.getGraphicsQueue());

    VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
		accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		accelerationDeviceAddressInfo.accelerationStructure = this->accelStruct;
		this->address = vkGetAccelerationStructureDeviceAddressKHR(this->appDevice.getLogicalDevice(), &accelerationDeviceAddressInfo);
  }
  
} // namespace nugiEngin 
