#pragma once

#include <stb_image.h>
#include <vulkan/vulkan.h>
#include <stdexcept>

#include "../buffer/buffer.hpp"
#include "../command/command_buffer.hpp"
#include "../image/image.hpp"

#include <memory>

namespace nugiEngine
{
  class EngineTexture
  {
    public:
      EngineTexture(EngineDevice &appDevice, const char* textureFileName);
      ~EngineTexture();

      VkDescriptorImageInfo getDescriptorInfo();

    private:
      EngineDevice &appDevice;
      std::unique_ptr<EngineImage> image;

      VkSampler sampler;
      uint32_t mipLevels;

      void createTextureImage(const char* textureFileName);
      void createTextureSampler();
  };
  
  
} // namespace nugiEngine
