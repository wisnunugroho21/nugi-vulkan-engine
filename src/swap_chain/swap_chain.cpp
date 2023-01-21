#include "swap_chain.hpp"
#include "../command/command_buffer.hpp"

// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace nugiEngine {

  EngineSwapChain::EngineSwapChain(EngineDevice &deviceRef, VkExtent2D extent)
    : device{deviceRef}, windowExtent{extent} {
      this->init();
  }

  EngineSwapChain::EngineSwapChain(EngineDevice &deviceRef, VkExtent2D extent, std::shared_ptr<EngineSwapChain> previous) 
    : device{deviceRef}, windowExtent{extent}, oldSwapChain{previous}  {
      this->init();
      this->oldSwapChain = nullptr;
  }

  EngineSwapChain::~EngineSwapChain() {
    if (swapChain != nullptr) {
      vkDestroySwapchainKHR(this->device.getLogicalDevice(), this->swapChain, nullptr);
      swapChain = nullptr;
    }
  }

  VkResult EngineSwapChain::acquireNextImage(uint32_t *imageIndex, VkFence *inFlightFence, VkSemaphore imageAvailableSemaphore) {
    vkWaitForFences(
      this->device.getLogicalDevice(),
      1,
      inFlightFence,
      VK_TRUE,
      std::numeric_limits<uint64_t>::max());

    VkResult result = vkAcquireNextImageKHR(
      this->device.getLogicalDevice(),
      this->swapChain,
      std::numeric_limits<uint64_t>::max(),
      imageAvailableSemaphore,  // must be a not signaled semaphore
      VK_NULL_HANDLE,
      imageIndex);

    return result;
  }

  VkResult EngineSwapChain::presentRenders(uint32_t *imageIndex, VkSemaphore* signalSemaphores) {
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { this->swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = imageIndex;

    auto result = vkQueuePresentKHR(this->device.getPresentQueue(), &presentInfo);
    return result;
  }

  void EngineSwapChain::init() {
    this->createSwapChain();
  }

  void EngineSwapChain::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = this->device.getSwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && 
      imageCount > swapChainSupport.capabilities.maxImageCount) 
    {
      imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = this->device.getSurface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    QueueFamilyIndices indices = this->device.findPhysicalQueueFamilies();
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily) {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 0;      // Optional
      createInfo.pQueueFamilyIndices = nullptr;  // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = this->oldSwapChain == nullptr ? VK_NULL_HANDLE : this->oldSwapChain->swapChain;

    if (vkCreateSwapchainKHR(device.getLogicalDevice(), &createInfo, nullptr, &this->swapChain) != VK_SUCCESS) {
      throw std::runtime_error("failed to create swap chain!");
    }

    std::vector<VkImage> tempSwapChainImages;
    vkGetSwapchainImagesKHR(this->device.getLogicalDevice(), swapChain, &imageCount, nullptr);
    tempSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(this->device.getLogicalDevice(), swapChain, &imageCount, tempSwapChainImages.data());

    this->swapChainImageFormat = surfaceFormat.format;
    this->swapChainExtent = extent;

    this->swapChainImages.clear();
    for (uint32_t i = 0; i < imageCount; i++) {
      auto swapChainImage = std::make_shared<EngineImage>(this->device, extent.width, extent.height, tempSwapChainImages[i], 1, this->swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
      this->swapChainImages.push_back(swapChainImage);
    }
  }

  VkSurfaceFormatKHR EngineSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat : availableFormats) {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
      {
        return availableFormat;
      }
    }

    return availableFormats[0];
  }

  VkPresentModeKHR EngineSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const auto &availablePresentMode : availablePresentModes) {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
        std::cout << "Present mode: Mailbox" << std::endl;
        return availablePresentMode;
      }
    }

    for (const auto &availablePresentMode : availablePresentModes) {
      if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
        std::cout << "Present mode: Immediate" << std::endl;
        return availablePresentMode;
      }
    }

    std::cout << "Present mode: V-Sync" << std::endl;
    return VK_PRESENT_MODE_FIFO_KHR;
  }

  VkExtent2D EngineSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
      return capabilities.currentExtent;
    } else {
      VkExtent2D actualExtent = windowExtent;
      actualExtent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, actualExtent.width)
      );
      actualExtent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, actualExtent.height)
      );

      return actualExtent;
    }
  }
}