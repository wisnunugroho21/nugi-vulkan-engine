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

  SwapChainSupportDetails getSwapChainSupport() { return this->querySwapChainSupport(physicalDevice); }
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
  QueueFamilyIndices findPhysicalQueueFamilies() { return this->findQueueFamilies(physicalDevice); }
  VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, 
    VkImageTiling tiling, VkFormatFeatureFlags features);

  void createImageWithInfo(
    const VkImageCreateInfo &imageInfo,
    VkMemoryPropertyFlags properties,
    VkImage &image,
    VkDeviceMemory &imageMemory
  );

  VkPhysicalDeviceProperties properties;

 private:
  void createInstance();
  void setupDebugMessenger();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createCommandPool();

  // helper functions
  bool isDeviceSuitable(VkPhysicalDevice device);
  std::vector<const char *> getRequiredExtensions();
  bool checkValidationLayerSupport();
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  void hasGflwRequiredInstanceExtensions();
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  EngineWindow &window;
  VkCommandPool commandPool;

  VkDevice device;
  VkSurfaceKHR surface;
  VkQueue graphicsQueue;
  VkQueue presentQueue;

  const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

}  // namespace lve