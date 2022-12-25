#include "bottom_level_acceleration_structure.hpp"

namespace nugiEngine {
	EngineBottomLevelAccelerationStructure::EngineBottomLevelAccelerationStructure(EngineDevice& appDevice, EngineDeviceProcedures& deviceProcedure, std::shared_ptr<EngineModel> model) 
		: appDevice{appDevice}, deviceProcedure{deviceProcedure} 
	{
		this->createBottomLevelAccelerationStructure(model);
	}

  void EngineBottomLevelAccelerationStructure::createBottomLevelAccelerationStructure(std::shared_ptr<EngineModel> model) {
    VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
		accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		accelerationStructureGeometry.geometry.triangles.vertexData.deviceAddress = model->getVertexBuffer()->getDeviceAddress();
		accelerationStructureGeometry.geometry.triangles.maxVertex = model->getVertextCount();
		accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
		accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		accelerationStructureGeometry.geometry.triangles.indexData.deviceAddress = model->getIndexBuffer()->getDeviceAddress();
		accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
		accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;
		accelerationStructureGeometry.geometry.triangles.transformData = {};

		VkAccelerationStructureGeometryKHR aabbGeometry = {};
		aabbGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		aabbGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		aabbGeometry.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
		aabbGeometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
		aabbGeometry.geometry.aabbs.data.deviceAddress = model->getAABBBuffer()->getDeviceAddress();
		aabbGeometry.geometry.aabbs.stride = sizeof(VkAabbPositionsKHR);

		std::vector<VkAccelerationStructureGeometryKHR> geometries{ accelerationStructureGeometry, aabbGeometry };
    uint32_t primitiveCount = static_cast<uint32_t>(geometries.size());

    // Get size info
		VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
		accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationStructureBuildGeometryInfo.geometryCount = static_cast<uint32_t>(geometries.size());
		accelerationStructureBuildGeometryInfo.pGeometries = geometries.data();

    VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
		accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		this->deviceProcedure.vkGetAccelerationStructureBuildSizesKHR(
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
		accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		this->deviceProcedure.vkCreateAccelerationStructureKHR(this->appDevice.getLogicalDevice(), &accelerationStructureCreateInfo, nullptr, &this->accelStruct);

		EngineBuffer stratchBuffer{
			this->appDevice,
			accelerationStructureBuildSizesInfo.buildScratchSize,
			static_cast<uint32_t>(1),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		};

		VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
		accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
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

		this->deviceProcedure.vkCmdBuildAccelerationStructuresKHR(
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
		this->address = this->deviceProcedure.vkGetAccelerationStructureDeviceAddressKHR(this->appDevice.getLogicalDevice(), &accelerationDeviceAddressInfo);
  }
  
} // namespace nugiEngin 
