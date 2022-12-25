#include "device_procedures.hpp"
#include <string>

namespace nugiEngine {

	template <class Func>
	Func GetProcedure(EngineDevice &appDevice, const char* const name)
	{
		const auto func = reinterpret_cast<Func>(vkGetDeviceProcAddr(appDevice.getLogicalDevice(), name));
		if (func == nullptr) {
			throw std::runtime_error(std::string("failed to get address of '") + name + "'");
		}

		return func;
	}

	EngineDeviceProcedures::EngineDeviceProcedures(EngineDevice &appDevice) : appDevice{appDevice} {
		this->vkCreateAccelerationStructureKHR = GetProcedure<PFN_vkCreateAccelerationStructureKHR>(this->appDevice, "vkCreateAccelerationStructureKHR");
		this->vkDestroyAccelerationStructureKHR = GetProcedure<PFN_vkDestroyAccelerationStructureKHR>(this->appDevice, "vkDestroyAccelerationStructureKHR");
		this->vkGetAccelerationStructureBuildSizesKHR = GetProcedure<PFN_vkGetAccelerationStructureBuildSizesKHR>(this->appDevice, "vkGetAccelerationStructureBuildSizesKHR");
		this->vkCmdBuildAccelerationStructuresKHR = GetProcedure<PFN_vkCmdBuildAccelerationStructuresKHR>(this->appDevice, "vkCmdBuildAccelerationStructuresKHR");
		this->vkCmdCopyAccelerationStructureKHR = GetProcedure<PFN_vkCmdCopyAccelerationStructureKHR>(this->appDevice, "vkCmdCopyAccelerationStructureKHR");
		this->vkCmdTraceRaysKHR = GetProcedure<PFN_vkCmdTraceRaysKHR>(this->appDevice, "vkCmdTraceRaysKHR");
		this->vkCreateRayTracingPipelinesKHR = GetProcedure<PFN_vkCreateRayTracingPipelinesKHR>(this->appDevice, "vkCreateRayTracingPipelinesKHR");
		this->vkGetRayTracingShaderGroupHandlesKHR = GetProcedure<PFN_vkGetRayTracingShaderGroupHandlesKHR>(this->appDevice, "vkGetRayTracingShaderGroupHandlesKHR");
		this->vkGetAccelerationStructureDeviceAddressKHR = GetProcedure<PFN_vkGetAccelerationStructureDeviceAddressKHR>(this->appDevice, "vkGetAccelerationStructureDeviceAddressKHR");
		this->vkCmdWriteAccelerationStructuresPropertiesKHR = GetProcedure<PFN_vkCmdWriteAccelerationStructuresPropertiesKHR>(this->appDevice, "vkCmdWriteAccelerationStructuresPropertiesKHR");
	}
}
