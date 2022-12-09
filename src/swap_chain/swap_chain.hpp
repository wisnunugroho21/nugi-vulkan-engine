#pragma once

#include "../device/device.hpp"
#include "../image/image.hpp"
#include "../renderpass/renderpass.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>
#include <memory>

namespace nugiEngine {

  class EngineSwapChain {
  public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    EngineSwapChain(EngineDevice &deviceRef, VkExtent2D windowExtent);
    EngineSwapChain(EngineDevice &deviceref, VkExtent2D windowExtent, std::shared_ptr<EngineSwapChain> previous);

    ~EngineSwapChain();

    EngineSwapChain(const EngineSwapChain &) = delete;
    EngineSwapChain& operator=(const EngineSwapChain &) = delete;

    VkFramebuffer getFrameBuffer(int index) const { return this->renderPass->getFramebuffers(index); }
    VkRenderPass getRenderPass() const { return this->renderPass->getRenderPass(); }
    VkImageView getImageView(int index) const { return this->swapChainImages[index]->getImageView(); }
    size_t imageCount() const { return this->swapChainImages.size(); }
    VkFormat getSwapChainImageFormat() const { return this->swapChainImageFormat; }
    VkExtent2D getSwapChainExtent() { return this->swapChainExtent; }
    uint32_t width() const { return this->swapChainExtent.width; }
    uint32_t height() const { return this->swapChainExtent.height; }

    float extentAspectRatio() {
      return static_cast<float>(this->swapChainExtent.width) / static_cast<float>(this->swapChainExtent.height);
    }
    VkFormat findDepthFormat();

    VkResult acquireNextImage(uint32_t *imageIndex);
    VkResult executeAndPresentRenders(const VkCommandBuffer *buffers, uint32_t *imageIndex);

    bool compareSwapFormat(const EngineSwapChain& swapChain) {
      return swapChain.swapChainDepthFormat == this->swapChainDepthFormat 
        && swapChain.swapChainImageFormat == this->swapChainImageFormat;
    }

  private:
    void init();
    void createSwapChain();
    void createColorResources();
    void createDepthResources();
    void createRenderPass();
    void createSyncObjects();

    // Helper functions
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    VkFormat swapChainImageFormat;
    VkFormat swapChainDepthFormat;
    VkExtent2D swapChainExtent;

    std::shared_ptr<EngineRenderPass> renderPass;

    std::vector<std::shared_ptr<EngineImage>> colorImages;
    std::vector<std::shared_ptr<EngineImage>> depthImages;
    std::vector<std::shared_ptr<EngineImage>> swapChainImages;

    EngineDevice &device;
    VkExtent2D windowExtent;

    VkSwapchainKHR swapChain;
    std::shared_ptr<EngineSwapChain> oldSwapChain;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;
  };
}
