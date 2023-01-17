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

    std::vector<std::shared_ptr<EngineImage>> getswapChainImages() const { return this->swapChainImages; }
    VkFormat getSwapChainImageFormat() const { return this->swapChainImageFormat; }
    VkExtent2D getSwapChainExtent() { return this->swapChainExtent; }
    size_t imageCount() const { return this->swapChainImages.size(); }
    uint32_t width() const { return this->swapChainExtent.width; }
    uint32_t height() const { return this->swapChainExtent.height; }

    float extentAspectRatio() {
      return static_cast<float>(this->swapChainExtent.width) / static_cast<float>(this->swapChainExtent.height);
    }

    VkResult acquireNextImage(uint32_t *imageIndex, VkFence *inFlightFence, VkSemaphore imageAvailableSemaphore);
    VkResult presentRenders(uint32_t *imageIndex, VkSemaphore* signalSemaphores);

    bool compareSwapFormat(const EngineSwapChain& swapChain) {
      return swapChain.swapChainImageFormat == this->swapChainImageFormat;
    }

  private:
    void init();
    void createSwapChain();

    // Helper functions
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    VkSwapchainKHR swapChain;
    std::shared_ptr<EngineSwapChain> oldSwapChain;

    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    std::vector<std::shared_ptr<EngineImage>> swapChainImages;

    EngineDevice &device;
    VkExtent2D windowExtent;

    size_t currentFrame = 0;
  };
}
