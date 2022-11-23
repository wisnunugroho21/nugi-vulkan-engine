#pragma once

#include "camera/camera.hpp"

#include <vulkan/vulkan.h>

namespace nugiEngine {
  struct FrameInfo {
  int frameIndex;
  float frameTime;
  VkCommandBuffer commandBuffer;
  EngineCamera &camera;
  VkDescriptorSet globalDescriptorSet;
};
  
} // namespace nugiEngine

