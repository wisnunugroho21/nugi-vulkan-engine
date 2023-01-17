#pragma once

#include "../image/image.hpp"
#include "../renderpass/renderpass.hpp"

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace nugiEngine {
  class EngineSwapChainSubRenderer {
    public:
      EngineSwapChainSubRenderer(EngineDevice &device, std::vector<std::shared_ptr<EngineImage>> swapChainImages, VkFormat swapChainImageFormat, int imageCount, int width, int height);
      std::shared_ptr<EngineRenderPass> getRenderPass() const { return this->renderPass; }

      void beginRenderPass(std::shared_ptr<EngineCommandBuffer> commandBuffer, int currentImageIndex);
			void endRenderPass(std::shared_ptr<EngineCommandBuffer> commandBuffer);
      
    private:
      int width, height;
      EngineDevice &device;

      std::vector<std::shared_ptr<EngineImage>> colorImages;
      std::vector<std::shared_ptr<EngineImage>> depthImages;
      std::vector<std::shared_ptr<EngineImage>> swapChainImages;
      
      std::shared_ptr<EngineRenderPass> renderPass;

      VkFormat findDepthFormat();

      void createColorResources(VkFormat swapChainImageFormat, int imageCount);
      void createDepthResources(int imageCount);
      void createRenderPass(VkFormat swapChainImageFormat, int imageCount);
  };
  
} // namespace nugiEngine
