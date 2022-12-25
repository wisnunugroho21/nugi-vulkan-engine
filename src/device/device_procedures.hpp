#pragma once

#include "../device/device.hpp"
#include <functional>

namespace nugiEngine
{
	class EngineDeviceProcedures {
		public:
			EngineDeviceProcedures(EngineDevice &appDevice);
			
			std::function<VkResult(
				VkDevice device,
				VkAccelerationStructureCreateInfoKHR* pCreateInfo,
				VkAllocationCallbacks* pAllocator,
				VkAccelerationStructureKHR* pAccelerationStructure)>
			vkCreateAccelerationStructureKHR;

			std::function<void(
				VkDevice device,
				VkAccelerationStructureKHR accelerationStructure,
				VkAllocationCallbacks* pAllocator)>
			vkDestroyAccelerationStructureKHR;

			std::function<void(
				VkDevice device,
				VkAccelerationStructureBuildTypeKHR buildType,
				VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
				uint32_t* pMaxPrimitiveCounts,
				VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo)>
			vkGetAccelerationStructureBuildSizesKHR;

			std::function<void(
				VkCommandBuffer commandBuffer,
				uint32_t infoCount,
				VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
				VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos)>
			vkCmdBuildAccelerationStructuresKHR;

			std::function<void(
				VkCommandBuffer commandBuffer,
				VkCopyAccelerationStructureInfoKHR* pInfo)>
			vkCmdCopyAccelerationStructureKHR;

			std::function<void(
				VkCommandBuffer commandBuffer,
				VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, 
				VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
				VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
				VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
				uint32_t width, 
				uint32_t height, 
				uint32_t depth)>
			vkCmdTraceRaysKHR;

			std::function<VkResult(
				VkDevice device,
				VkDeferredOperationKHR deferredOperation,
				VkPipelineCache pipelineCache,
				uint32_t createInfoCount,
				VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
				VkAllocationCallbacks* pAllocator,
				VkPipeline* pPipelines)>
			vkCreateRayTracingPipelinesKHR;

			std::function<VkResult(
				VkDevice device,
				VkPipeline pipeline,
				uint32_t firstGroup,
				uint32_t groupCount,
				size_t dataSize,
				void* pData)>
			vkGetRayTracingShaderGroupHandlesKHR;

			std::function<VkDeviceAddress(
				VkDevice device, 
				VkAccelerationStructureDeviceAddressInfoKHR* pInfo)>
			vkGetAccelerationStructureDeviceAddressKHR;

			std::function<void(
				VkCommandBuffer commandBuffer,
				uint32_t accelerationStructureCount,
				VkAccelerationStructureKHR* pAccelerationStructures,
				VkQueryType queryType,
				VkQueryPool queryPool,
				uint32_t firstQuery)>
			vkCmdWriteAccelerationStructuresPropertiesKHR;
			
		private:
			EngineDevice& appDevice;
		};
}
