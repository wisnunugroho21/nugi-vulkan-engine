#include "renderpass.hpp"

#include <array>

namespace nugiEngine {
  EngineRenderPass::Builder::Builder(EngineDevice &appDevice, int width, int height) : 
    appDevice{appDevice}, width{width}, height{height} 
  {

  }

  EngineRenderPass::Builder EngineRenderPass::Builder::addSubpass(VkSubpassDescription subpass) {
    this->subpasses.push_back(subpass);
    return *this;
  }

  EngineRenderPass::Builder EngineRenderPass::Builder::addAttachments(VkAttachmentDescription attachment) {
    this->attachments.push_back(attachment);
    return *this;
  }

  EngineRenderPass::Builder EngineRenderPass::Builder::addDependency(VkSubpassDependency dependency) {
    this->dependencies.push_back(dependency);
    return *this;
  }

  EngineRenderPass::Builder EngineRenderPass::Builder::addViewImages(std::vector<VkImageView> viewImages) {
    this->viewImages.push_back(viewImages);
    return *this;
  }

  std::shared_ptr<EngineRenderPass> EngineRenderPass::Builder::build() {
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(this->attachments.size());
    renderPassInfo.pAttachments = this->attachments.data();
    renderPassInfo.subpassCount = static_cast<uint32_t>(this->subpasses.size());
    renderPassInfo.pSubpasses = this->subpasses.data();
    renderPassInfo.dependencyCount = static_cast<uint32_t>(this->dependencies.size());
    renderPassInfo.pDependencies = this->dependencies.data();

    return std::make_shared<EngineRenderPass>(this->appDevice, this->viewImages, renderPassInfo, this->width, this->height);
  }

  EngineRenderPass::EngineRenderPass(EngineDevice &appDevice, std::vector<std::vector<VkImageView>> viewImages, VkRenderPassCreateInfo renderPassInfo, int width, int height) : appDevice{appDevice} {
    this->createRenderPass(renderPassInfo);
    this->createFramebuffers(viewImages, width, height);
  }

  EngineRenderPass::~EngineRenderPass() {
    for (auto framebuffer : this->framebuffers) {
      vkDestroyFramebuffer(this->appDevice.getLogicalDevice(), framebuffer, nullptr);
    }

    vkDestroyRenderPass(this->appDevice.getLogicalDevice(), this->renderPass, nullptr);
  }
  
  void EngineRenderPass::createRenderPass(VkRenderPassCreateInfo renderPassInfo) {
    if (vkCreateRenderPass(this->appDevice.getLogicalDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
      throw std::runtime_error("failed to create render pass!");
    }
  }

  void EngineRenderPass::createFramebuffers(std::vector<std::vector<VkImageView>> viewImages, int width, int height) {
    this->framebuffers.resize(viewImages.size());

    for (size_t i = 0; i < viewImages.size(); i++) {
      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = this->renderPass;
      framebufferInfo.attachmentCount = static_cast<uint32_t>(viewImages[i].size());
      framebufferInfo.pAttachments = viewImages[i].data();
      framebufferInfo.width = width;
      framebufferInfo.height = height;
      framebufferInfo.layers = 1;

      if (vkCreateFramebuffer(
        this->appDevice.getLogicalDevice(),
        &framebufferInfo,
        nullptr,
        &this->framebuffers[i]) != VK_SUCCESS) 
      {
        throw std::runtime_error("failed to create framebuffer!");
      }
    }
  }
} // namespace nugiEngine
