#pragma once

#include "../device/device.hpp"
#include "../image/image.hpp"

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace nugiEngine {
  class EngineRenderPass {
    public:
      class Builder {
        public:
          Builder(EngineDevice &appDevice, int width, int height);

          Builder addSubpass(VkSubpassDescription subpass);
          Builder addAttachments(VkAttachmentDescription attachment);
          Builder addDependency(VkSubpassDependency dependency);
          Builder addViewImages(std::vector<VkImageView> viewImages);

          std::shared_ptr<EngineRenderPass> build();

        private:
          int width, height;
          EngineDevice &appDevice;
          std::vector<VkSubpassDescription> subpasses;
          std::vector<VkAttachmentDescription> attachments;
          std::vector<VkSubpassDependency> dependencies;
          std::vector<std::vector<VkImageView>> viewImages;
      };

      EngineRenderPass(EngineDevice &appDevice, std::vector<std::vector<VkImageView>> viewImages, VkRenderPassCreateInfo renderPassInfo, int width, int height);
      ~EngineRenderPass();

      EngineRenderPass(const EngineRenderPass &) = delete;
      EngineRenderPass& operator=(const EngineRenderPass &) = delete;

      VkFramebuffer getFramebuffers(int index) { return this->framebuffers[index]; }
      VkRenderPass getRenderPass() { return this->renderPass; }

    private:
      EngineDevice &appDevice;

      std::vector<VkFramebuffer> framebuffers;
      VkRenderPass renderPass;

      void createRenderPass(VkRenderPassCreateInfo renderPassInfo);
      void createFramebuffers(std::vector<std::vector<VkImageView>> viewImages, int width, int height);
  };
} // namespace nugiEngin 


