#include "image.hpp"

namespace nugiEngine {
  EngineImage::EngineImage(EngineDevice &appDevice, uint32_t width, uint32_t height, VkFormat format,
    VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags) : appDevice{appDevice} 
  {
    this->createImage(width, height, format, tiling, usage, properties);
    this->createImageView(format, aspectFlags);

    this->isImageCreatedByUs = false;
  }

  EngineImage::EngineImage(EngineDevice &appDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) : appDevice{appDevice} {
    this->image = image;
    this->createImageView(format, aspectFlags);

    this->isImageCreatedByUs = false;
  }

  EngineImage::~EngineImage() {
    vkDestroyImageView(this->appDevice.getLogicalDevice(), this->imageView, nullptr);

    if (this->isImageCreatedByUs) {
      vkDestroyImage(this->appDevice.getLogicalDevice(), this->image, nullptr);
      vkFreeMemory(this->appDevice.getLogicalDevice(), this->imageMemory, nullptr);
    }
  }

  void EngineImage::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(this->appDevice.getLogicalDevice(), &imageInfo, nullptr, &this->image) != VK_SUCCESS) {
      throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(this->appDevice.getLogicalDevice(), this->image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = this->appDevice.findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(this->appDevice.getLogicalDevice(), &allocInfo, nullptr, &this->imageMemory) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate image memory!");
    }

    if (vkBindImageMemory(this->appDevice.getLogicalDevice(), this->image, this->imageMemory, 0) != VK_SUCCESS) {
      throw std::runtime_error("failed to bind image memory!");
    }
  }

  void EngineImage::createImageView(VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = this->image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(appDevice.getLogicalDevice(), &viewInfo, nullptr, &this->imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
  }
  
} // namespace nugiEngine
