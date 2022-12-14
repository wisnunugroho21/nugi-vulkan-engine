#pragma once

#include "../image/image.hpp"
#include "../renderpass/renderpass.hpp"

#include <vector>
#include <memory>

namespace nugiEngine {
  class EngineShadowMapSubRenderer
  {
    public:
      EngineShadowMapSubRenderer(EngineDevice &device, int imageCount, int width, int height);
      std::shared_ptr<EngineRenderPass> getRenderPass() const { return this->renderPass; }

      std::vector<std::shared_ptr<EngineImage>> getDepthImages() { return this->depthImages; }

      void beginRenderPass(std::shared_ptr<EngineCommandBuffer> commandBuffer, int currentImageIndex);
      void endRenderPass(std::shared_ptr<EngineCommandBuffer> commandBuffer);

    private:
      int width, height;
      EngineDevice &device;

      std::vector<std::shared_ptr<EngineImage>> depthImages;
      std::shared_ptr<EngineRenderPass> renderPass;

      VkFormat findDepthFormat();
      void createDepthResources(int imageCount);
      void createRenderPass(int imageCount);
  };
  
} // namespace nugiEngine
