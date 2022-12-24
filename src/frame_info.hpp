#pragma once

#include "camera/camera.hpp"

#include <vulkan/vulkan.h>

namespace nugiEngine {
  struct FrameInfo {
    int frameIndex;
    int imageIndex;
    float frameTime;
    EngineCamera &camera;
  };
  
} // namespace nugiEngine

