#pragma once

#include "camera/camera.hpp"

#include <vulkan/vulkan.h>

namespace nugiEngine {
  struct FrameInfo {
    int frameIndex;
    float frameTime;
    float aspect;
    EngineCamera &camera;
  };
  
} // namespace nugiEngine

