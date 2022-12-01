#pragma once

#include "../window/window.hpp"

// std lib headers
#include <string>
#include <vector>

namespace nugiEngine {
  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;
    bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
  };

  class EngineDevice {
    public:
    #ifdef NDEBUG
      const bool enableValidationLayers = false;
    #else
      const bool enableValidationLayers = true;
    #endif

      EngineDevice(EngineWindow &window);
      ~EngineDevice();

      // Not copyable or movable
      EngineDevice(const EngineDevice &) = delete;
      void operator=(const EngineDevice &) = delete;
      EngineDevice(EngineDevice &&) = delete;
      EngineDevice &operator=(EngineDevice &&) = delete;

      VkCommandPool getCommandPool() { return this->commandPool; }
      VkDevice getLogicalDevice() { return this->device; }
      VkSurfaceKHR getSurface() { return this->surface; }
      VkQueue getGraphicsQueue() { return this->graphicsQueue; }
      VkQueue getPresentQueue() { return this->presentQueue; }
      VkPhysicalDeviceProperties getProperties() { return this->properties; }

      SwapChainSupportDetails getSwapChainSupport() { return this->querySwapChainSupport(this->physicalDevice); }
      QueueFamilyIndices findPhysicalQueueFamilies() { return this->findQueueFamilies(this->physicalDevice); }
      uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
      VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    private:
      // creation function
      void createInstance();
      void setupDebugMessenger();
      void createSurface();
      void pickPhysicalDevice();
      void createLogicalDevice();
      void createCommandPool();

      // helper creation functions
      bool isDeviceSuitable(VkPhysicalDevice device);
      bool checkDeviceExtensionSupport(VkPhysicalDevice device);
      bool checkValidationLayerSupport();
      void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
      void hasGflwRequiredInstanceExtensions();
      std::vector<const char *> getRequiredExtensions();
      QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
      SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

      VkInstance instance;
      VkDebugUtilsMessengerEXT debugMessenger;

      VkDevice device;
      VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
      VkPhysicalDeviceProperties properties;

      EngineWindow &window;
      VkSurfaceKHR surface;

      VkCommandPool commandPool;
      VkQueue graphicsQueue;
      VkQueue presentQueue;

      const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
      const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  };

}  // namespace lve