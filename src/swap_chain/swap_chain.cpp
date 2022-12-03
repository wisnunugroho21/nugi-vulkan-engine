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

    for (auto framebuffer : this->swapChainFramebuffers) {
      vkDestroyFramebuffer(this->device.getLogicalDevice(), framebuffer, nullptr);
    }

    vkDestroyRenderPass(this->device.getLogicalDevice(), this->renderPass, nullptr);

    // cleanup synchronization objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroySemaphore(this->device.getLogicalDevice(), this->renderFinishedSemaphores[i], nullptr);
      vkDestroySemaphore(this->device.getLogicalDevice(), this->imageAvailableSemaphores[i], nullptr);
      vkDestroyFence(this->device.getLogicalDevice(), this->inFlightFences[i], nullptr);
    }
  }

  VkResult EngineSwapChain::acquireNextImage(uint32_t *imageIndex) {
    vkWaitForFences(
      this->device.getLogicalDevice(),
      1,
      &this->inFlightFences[this->currentFrame],
      VK_TRUE,
      std::numeric_limits<uint64_t>::max());

    VkResult result = vkAcquireNextImageKHR(
      this->device.getLogicalDevice(),
      this->swapChain,
      std::numeric_limits<uint64_t>::max(),
      this->imageAvailableSemaphores[this->currentFrame],  // must be a not signaled semaphore
      VK_NULL_HANDLE,
      imageIndex);

    return result;
  }

  VkResult EngineSwapChain::executeAndPresentRenders(const VkCommandBuffer *buffers, uint32_t *imageIndex) {
    if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
      vkWaitForFences(device.getLogicalDevice(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
    }

    imagesInFlight[*imageIndex] = inFlightFences[currentFrame];
    vkResetFences(device.getLogicalDevice(), 1, &inFlightFences[currentFrame]);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffers;

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    if (vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
      throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = imageIndex;

    auto result = vkQueuePresentKHR(device.getPresentQueue(), &presentInfo);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
  }

  void EngineSwapChain::init() {
    this->createSwapChain();
    this->createRenderPass();
    this->createColorResources();
    this->createDepthResources();
    this->createFramebuffers();
    this->createSyncObjects();
  }

  void EngineSwapChain::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = this->device.getSwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
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
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

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

    // we only specified a minimum number of images in the swap chain, so the implementation is
    // allowed to create a swap chain with more. That's why we'll first query the final number of
    // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
    // retrieve the handles.
    std::vector<VkImage> tempSwapChainImages;
    vkGetSwapchainImagesKHR(this->device.getLogicalDevice(), swapChain, &imageCount, nullptr);
    tempSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(this->device.getLogicalDevice(), swapChain, &imageCount, tempSwapChainImages.data());

    this->swapChainImageFormat = surfaceFormat.format;
    this->swapChainExtent = extent;

    this->swapChainImages.clear();
    for (uint32_t i = 0; i < imageCount; i++) {
      auto swapChainImage = std::make_shared<EngineImage>(this->device, tempSwapChainImages[i], 1, this->swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
      this->swapChainImages.push_back(swapChainImage);
    }
  }

  void EngineSwapChain::createRenderPass() {
    auto msaaSamples = this->device.getMSAASamples();

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = this->findDepthFormat();
    depthAttachment.samples = msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = this->getSwapChainImageFormat();
    colorAttachment.samples = msaaSamples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorResolveAttachment{};
    colorResolveAttachment.format = this->getSwapChainImageFormat();
    colorResolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorResolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorResolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorResolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorResolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorResolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorResolveAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorResolveAttachmentRef{};
    colorResolveAttachmentRef.attachment = 2;
    colorResolveAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorResolveAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstSubpass = 0;
    dependency.dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorResolveAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(this->device.getLogicalDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
      throw std::runtime_error("failed to create render pass!");
    }
  }

  void EngineSwapChain::createFramebuffers() {
    this->swapChainFramebuffers.resize(this->imageCount());
    for (size_t i = 0; i < this->imageCount(); i++) {
      std::array<VkImageView, 3> attachments = {
        this->colorImages[i]->getImageView(), 
        this->depthImages[i]->getImageView(),
        this->swapChainImages[i]->getImageView()
      };

      VkExtent2D swapChainExtent = this->getSwapChainExtent();
      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = renderPass;
      framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
      framebufferInfo.pAttachments = attachments.data();
      framebufferInfo.width = swapChainExtent.width;
      framebufferInfo.height = swapChainExtent.height;
      framebufferInfo.layers = 1;

      if (vkCreateFramebuffer(
        this->device.getLogicalDevice(),
        &framebufferInfo,
        nullptr,
        &this->swapChainFramebuffers[i]) != VK_SUCCESS) 
      {
        throw std::runtime_error("failed to create framebuffer!");
      }
    }
  }

  void EngineSwapChain::createColorResources() {
    VkFormat colorFormat = this->swapChainImageFormat;

    auto msaaSamples = this->device.getMSAASamples();
    this->colorImages.clear();

    for (int i = 0; i < this->imageCount(); i++) {
      auto colorImage = std::make_shared<EngineImage>(
        this->device, this->swapChainExtent.width, this->swapChainExtent.height, 1, msaaSamples, colorFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT
      );

      this->colorImages.push_back(colorImage);
    }
  }

  void EngineSwapChain::createDepthResources() {
    VkFormat depthFormat = this->findDepthFormat();
    this->swapChainDepthFormat = depthFormat;
    
    this->depthImages.clear();

    for (int i = 0; i < this->imageCount(); i++) {
      auto depthImage = std::make_shared<EngineImage>(
        this->device, this->swapChainExtent.width, this->swapChainExtent.height, 1, VK_SAMPLE_COUNT_1_BIT, depthFormat, 
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT
      );

      this->depthImages.push_back(depthImage);
    }
  }

  void EngineSwapChain::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      if (vkCreateSemaphore(this->device.getLogicalDevice(), &semaphoreInfo, nullptr, &this->imageAvailableSemaphores[i]) != VK_SUCCESS ||
          vkCreateSemaphore(this->device.getLogicalDevice(), &semaphoreInfo, nullptr, &this->renderFinishedSemaphores[i]) != VK_SUCCESS ||
          vkCreateFence(this->device.getLogicalDevice(), &fenceInfo, nullptr, &this->inFlightFences[i]) != VK_SUCCESS) 
      {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
      }
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

  VkFormat EngineSwapChain::findDepthFormat() {
    return this->device.findSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  }
}