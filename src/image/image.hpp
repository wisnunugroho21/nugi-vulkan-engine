#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>

#include "../buffer/buffer.hpp"
#include "../command/command_buffer.hpp"

namespace nugiEngine
{
  class EngineImage
  {
    public:
      EngineImage(EngineDevice &appDevice, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, 
        VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
        VkImageAspectFlags aspectFlags);
      EngineImage(EngineDevice &appDevice, uint32_t width, uint32_t height, VkImage image, uint32_t mipLevels, VkFormat format, VkImageAspectFlags aspectFlags);
      ~EngineImage();

      VkImage getImage() const { return this->image; }
      VkImageView getImageView() const { return this->imageView; }
      VkDeviceMemory getImageMemory() const { return this->imageMemory; }

      void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
      void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, 
        std::shared_ptr<EngineCommandBuffer> commandBuffer = nullptr);

      void copyImageFromOther(std::shared_ptr<EngineImage> srcImage, VkImageLayout srcLayout, VkImageLayout dstLayout, std::shared_ptr<EngineCommandBuffer> commandBuffer = nullptr);
      void copyImageToOther(std::shared_ptr<EngineImage> dstImage, VkImageLayout srcLayout, VkImageLayout dstLayout, std::shared_ptr<EngineCommandBuffer> commandBuffer = nullptr);

      void generateMipMap();

    private:
      EngineDevice &appDevice;

      VkImage image;
      VkImageView imageView;
      VkDeviceMemory imageMemory;
      VkFormat format;
      VkImageAspectFlags aspectFlags;
      
      uint32_t width;
      uint32_t height;
      uint32_t mipLevels;
      bool isImageCreatedByUs = false;

      void createImage(VkSampleCountFlagBits numSamples, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
      void createImageView();
  };
  
  
} // namespace nugiEngine
