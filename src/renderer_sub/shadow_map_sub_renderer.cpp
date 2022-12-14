#include "shadow_map_sub_renderer.hpp"

#include <assert.h>
#include <array>

namespace nugiEngine {
  EngineShadowMapSubRenderer::EngineShadowMapSubRenderer(EngineDevice &device, int imageCount, int width, int height) 
    : device{device}, width{width}, height{height}
  {
    this->createDepthResources(imageCount);
    this->createRenderPass(imageCount);
  }

  void EngineShadowMapSubRenderer::createDepthResources(int imageCount) {
    VkFormat depthFormat = this->findDepthFormat();
    
    auto msaaSamples = this->device.getMSAASamples();
    this->depthImages.clear();

    for (int i = 0; i < imageCount; i++) {
      auto depthImage = std::make_shared<EngineImage>(
        this->device, this->width, this->height, 1, msaaSamples, depthFormat, 
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT
      );

      this->depthImages.push_back(depthImage);
    }
  }

  void EngineShadowMapSubRenderer::createRenderPass(int imageCount) {
    auto msaaSamples = this->device.getMSAASamples();

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = this->findDepthFormat();
    depthAttachment.samples = msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

		EngineRenderPass::Builder renderPassBuilder = EngineRenderPass::Builder(this->device, this->width, this->height)
			.addAttachments(depthAttachment)
			.addSubpass(subpass);

    for (int i = 0; i < imageCount; i++) {
			renderPassBuilder.addViewImages({
        this->depthImages[i]->getImageView()
      });
    }

		this->renderPass = renderPassBuilder.build();
  }

  VkFormat EngineShadowMapSubRenderer::findDepthFormat() {
    return this->device.findSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  }

  void EngineShadowMapSubRenderer::beginRenderPass(std::shared_ptr<EngineCommandBuffer> commandBuffer, int currentImageIndex) {
		VkRenderPassBeginInfo renderBeginInfo{};
		renderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderBeginInfo.renderPass = this->getRenderPass()->getRenderPass();
		renderBeginInfo.framebuffer = this->getRenderPass()->getFramebuffers(currentImageIndex);
		renderBeginInfo.renderArea.offset = {0, 0};
		renderBeginInfo.renderArea.extent = { static_cast<uint32_t>(this->width), static_cast<uint32_t>(this->height) };

		std::array<VkClearValue, 1> clearValues{};
		clearValues[0].depthStencil = {1.0f, 0};
		renderBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer->getCommandBuffer(), &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<uint32_t>(this->width);
		viewport.height = static_cast<uint32_t>(this->height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{{0, 0}, { static_cast<uint32_t>(this->width), static_cast<uint32_t>(this->height) }};
		vkCmdSetViewport(commandBuffer->getCommandBuffer(), 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer->getCommandBuffer(), 0, 1, &scissor);
	}

	void EngineShadowMapSubRenderer::endRenderPass(std::shared_ptr<EngineCommandBuffer> commandBuffer) {
		vkCmdEndRenderPass(commandBuffer->getCommandBuffer());
	}
  
} // namespace nugiEngine
