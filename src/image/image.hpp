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
      VkImageLayout getLayout() const { return this->layout; }
      
      VkImageAspectFlags getAspectFlag() { return this->aspectFlags; }
      uint32_t getMipLevels() { return this->mipLevels; }

      VkDescriptorImageInfo getDescriptorInfo(VkImageLayout desiredImageLayout);

      void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
      void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, 
        std::shared_ptr<EngineCommandBuffer> commandBuffer = nullptr);

      void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, 
        VkAccessFlags srcAccess, VkAccessFlags dstAccess, std::shared_ptr<EngineCommandBuffer> commandBuffer = nullptr, EngineDevice *appDevice = nullptr);

      void copyImageFromOther(std::shared_ptr<EngineImage> srcImage, VkImageLayout srcLayout, VkImageLayout dstLayout, std::shared_ptr<EngineCommandBuffer> commandBuffer = nullptr);
      void copyImageToOther(std::shared_ptr<EngineImage> dstImage, VkImageLayout srcLayout, VkImageLayout dstLayout, std::shared_ptr<EngineCommandBuffer> commandBuffer = nullptr);

      void generateMipMap();

      static void transitionImageLayout(std::vector<std::shared_ptr<EngineImage>> images, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStage, 
        VkPipelineStageFlags dstStage, std::shared_ptr<EngineCommandBuffer> commandBuffer = nullptr, EngineDevice *appDevice = nullptr);

      static void transitionImageLayout(std::vector<std::shared_ptr<EngineImage>> images, VkImageLayout oldLayout, VkImageLayout newLayout, 
        VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkAccessFlags srcAccess, VkAccessFlags dstAccess,
        std::shared_ptr<EngineCommandBuffer> commandBuffer = nullptr, EngineDevice *appDevice = nullptr);

    private:
      EngineDevice &appDevice;

      VkImage image;
      VkImageView imageView;
      VkDeviceMemory imageMemory;
      VkFormat format;
      VkImageAspectFlags aspectFlags;
      VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
      
      uint32_t width;
      uint32_t height;
      uint32_t mipLevels;
      bool isImageCreatedByUs = false;

      void createImage(VkSampleCountFlagBits numSamples, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
      void createImageView();
  };
  
  
} // namespace nugiEngine
