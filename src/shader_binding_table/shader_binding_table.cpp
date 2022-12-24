#include "shader_binding_table.hpp"

namespace nugiEngine {
  void EngineShaderBindingTable::createShaderBindingTables(std::vector<VkRayTracingShaderGroupCreateInfoKHR> shader_groups) {
    const uint32_t handle_size = this->appDevice.getRayTracingProperties().shaderGroupHandleSize;
    const uint32_t handle_alignment = this->appDevice.getRayTracingProperties().shaderGroupHandleAlignment;

    const uint32_t handle_size_aligned = EngineShaderBindingTable::aligned_size(handle_size, handle_alignment);
    const uint32_t group_count = static_cast<uint32_t>(shader_groups.size());

    const uint32_t sbt_size = group_count * handle_size_aligned;

    this->raygenSBTBuffer = std::make_unique<EngineBuffer>(
      this->appDevice,
			handle_size_aligned,
			group_count,
			VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    this->missgenSBTBuffer = std::make_unique<EngineBuffer>(
      this->appDevice,
			handle_size_aligned,
			group_count,
			VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    
    this->hitgenSBTBuffer = std::make_unique<EngineBuffer>(
      this->appDevice,
			handle_size_aligned,
			group_count,
			VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    // Copy the pipeline's shader handles into a host buffer
    std::vector<uint8_t> shader_handle_storage(sbt_size);

    if (vkGetRayTracingShaderGroupHandlesKHR(get_device().get_handle(), pipeline, 0, group_count, sbt_size, shader_handle_storage.data()) != VK_SUCCESS) {
      throw std::runtime_error("failed to create ray tracing shader group handler");
    }

    // Copy the shader handles from the host buffer to the binding tables
    uint8_t *data = static_cast<uint8_t *>(raygen_shader_binding_table->map());
    memcpy(data, shader_handle_storage.data(), handle_size);
    data = static_cast<uint8_t *>(miss_shader_binding_table->map());
    memcpy(data, shader_handle_storage.data() + handle_size_aligned, handle_size);
    data = static_cast<uint8_t *>(hit_shader_binding_table->map());
    memcpy(data, shader_handle_storage.data() + handle_size_aligned * 2, handle_size);
    raygen_shader_binding_table->unmap();
    miss_shader_binding_table->unmap();
    hit_shader_binding_table->unmap();
  }
  
} // namespace nugiEngine
