/*
 * Encapsulates a vulkan buffer
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */
 
#include "buffer.hpp"
#include "../command/command_buffer.hpp"
 
// std
#include <cassert>
#include <cstring>
 
namespace nugiEngine {
  /**
   * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
   *
   * @param instanceSize The size of an instance
   * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg
   * minUniformBufferOffsetAlignment)
   *
   * @return VkResult of the buffer mapping call
   */
  VkDeviceSize EngineBuffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
    if (minOffsetAlignment > 0) {
      return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }
    return instanceSize;
  }
  
  EngineBuffer::EngineBuffer(
      EngineDevice &device,
      VkDeviceSize instanceSize,
      uint32_t instanceCount,
      VkBufferUsageFlags usageFlags,
      VkMemoryPropertyFlags memoryPropertyFlags,
      VkDeviceSize minOffsetAlignment
    )
      : engineDevice{device},
        instanceSize{instanceSize},
        instanceCount{instanceCount},
        usageFlags{usageFlags},
        memoryPropertyFlags{memoryPropertyFlags} 
  {
    this->alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
    this->bufferSize = alignmentSize * instanceCount;

    this->createBuffer(bufferSize, usageFlags, memoryPropertyFlags);
  }
  
  EngineBuffer::~EngineBuffer() {
    this->unmap();
    vkDestroyBuffer(this->engineDevice.getLogicalDevice(), this->buffer, nullptr);
    vkFreeMemory(this->engineDevice.getLogicalDevice(), this->memory, nullptr);
  }
  
  /**
   * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
   *
   * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
   * buffer range.
   * @param offset (Optional) Byte offset from beginning
   *
   * @return VkResult of the buffer mapping call
   */
  VkResult EngineBuffer::map(VkDeviceSize size, VkDeviceSize offset) {
    assert(this->buffer && this->memory && "Called map on buffer before create");
    return vkMapMemory(this->engineDevice.getLogicalDevice(), this->memory, offset, size, 0, &this->mapped);
  }
  
  /**
   * Unmap a mapped memory range
   *
   * @note Does not return a result as vkUnmapMemory can't fail
   */
  void EngineBuffer::unmap() {
    if (this->mapped) {
      vkUnmapMemory(this->engineDevice.getLogicalDevice(), this->memory);
      this->mapped = nullptr;
    }
  }
  
  /**
   * Copies the specified data to the mapped buffer. Default value writes whole buffer range
   *
   * @param data Pointer to the data to copy
   * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
   * range.
   * @param offset (Optional) Byte offset from beginning of mapped region
   *
   */
  void EngineBuffer::writeToBuffer(void *data, VkDeviceSize size, VkDeviceSize offset) {
    assert(this->mapped && "Cannot copy to unmapped buffer");
  
    if (size == VK_WHOLE_SIZE) {
      memcpy(this->mapped, data, this->bufferSize);
    } else {
      char *memOffset = (char *)this->mapped;
      memOffset += offset;
      memcpy(memOffset, data, size);
    }
  }
  
  /**
   * Flush a memory range of the buffer to make it visible to the device
   *
   * @note Only required for non-coherent memory
   *
   * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
   * complete buffer range.
   * @param offset (Optional) Byte offset from beginning
   *
   * @return VkResult of the flush call
   */
  VkResult EngineBuffer::flush(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = this->memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(this->engineDevice.getLogicalDevice(), 1, &mappedRange);
  }
  
  /**
   * Invalidate a memory range of the buffer to make it visible to the host
   *
   * @note Only required for non-coherent memory
   *
   * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
   * the complete buffer range.
   * @param offset (Optional) Byte offset from beginning
   *
   * @return VkResult of the invalidate call
   */
  VkResult EngineBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = this->memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkInvalidateMappedMemoryRanges(this->engineDevice.getLogicalDevice(), 1, &mappedRange);
  }
  
  /**
   * Create a buffer info descriptor
   *
   * @param size (Optional) Size of the memory range of the descriptor
   * @param offset (Optional) Byte offset from beginning
   *
   * @return VkDescriptorBufferInfo of specified offset and range
   */
  VkDescriptorBufferInfo EngineBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
    return VkDescriptorBufferInfo{
        this->buffer,
        offset,
        size,
    };
  }
  
  /**
   * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
   *
   * @param data Pointer to the data to copy
   * @param index Used in offset calculation
   *
   */
  void EngineBuffer::writeToIndex(void *data, int index) {
    this->writeToBuffer(data, this->instanceSize, index * this->alignmentSize);
  }
  
  /**
   *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
   *
   * @param index Used in offset calculation
   *
   */
  VkResult EngineBuffer::flushIndex(int index) { 
    return this->flush(this->alignmentSize, index * this->alignmentSize); 
  }
  
  /**
   * Create a buffer info descriptor
   *
   * @param index Specifies the region given by index * alignmentSize
   *
   * @return VkDescriptorBufferInfo for instance at index
   */
  VkDescriptorBufferInfo EngineBuffer::descriptorInfoForIndex(int index) {
    return this->descriptorInfo(this->alignmentSize, index * this->alignmentSize);
  }
  
  /**
   * Invalidate a memory range of the buffer to make it visible to the host
   *
   * @note Only required for non-coherent memory
   *
   * @param index Specifies the region to invalidate: index * alignmentSize
   *
   * @return VkResult of the invalidate call
   */
  VkResult EngineBuffer::invalidateIndex(int index) {
    return this->invalidate(this->alignmentSize, index * this->alignmentSize);
  }

  void EngineBuffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(this->engineDevice.getLogicalDevice(), &bufferInfo, nullptr, &this->buffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to create vertex buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(this->engineDevice.getLogicalDevice(), this->buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = this->engineDevice.findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(this->engineDevice.getLogicalDevice(), &allocInfo, nullptr, &this->memory) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(this->engineDevice.getLogicalDevice(), this->buffer, this->memory, 0);
  }

  void EngineBuffer::copyBuffer(VkBuffer srcBuffer, VkDeviceSize size) {
    EngineCommandBuffer commandBuffer{this->engineDevice};
    commandBuffer.beginSingleTimeCommand();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;  // Optional
    copyRegion.dstOffset = 0;  // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer.getCommandBuffer(), srcBuffer, this->buffer, 1, &copyRegion);

    commandBuffer.endCommand();
    commandBuffer.submitCommand(this->engineDevice.getGraphicsQueue());
  }

  void EngineBuffer::copyBufferToImage(VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) {
    EngineCommandBuffer commandBuffer{this->engineDevice};
    commandBuffer.beginSingleTimeCommand();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(
      commandBuffer.getCommandBuffer(),
      this->buffer,
      image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1,
      &region
    );

    commandBuffer.endCommand();
    commandBuffer.submitCommand(this->engineDevice.getGraphicsQueue());
  }
  
 
}  // namespace lve