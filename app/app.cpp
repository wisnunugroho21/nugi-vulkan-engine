#include "app.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <string>

namespace nugiEngine {

	struct SimplePushConstantData {
		glm::mat2 transform{1.0f};
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};
	

	EngineApp::EngineApp() {
		this->loadObjects();
		this->createPipelineLayout();
		this->recreateSwapChain();
		this->createCommandBuffers();
	}

	EngineApp::~EngineApp() {
		vkDestroyPipelineLayout(this->device.device(), this->pipelineLayout, nullptr);
	}

	void EngineApp::run() {
		while (!this->window.shouldClose()) {
			this->window.pollEvents();
			this->drawFrame();
		}

		vkDeviceWaitIdle(this->device.device());
	}

	void EngineApp::loadObjects() {
		std::vector<Vertex> vertices {
			{{ 0.0f, -0.5f }, {1.0f, 0.0f, 0.0f}},
			{{ 0.5f, 0.5f }, {0.0f, 1.0f, 0.0f}},
			{{ -0.5f, 0.5f }, {0.0f, 0.0f, 1.0f}}
		};

		auto model = std::make_shared<EngineModel>(this->device, vertices); 

		auto triangle = EngineGameObject::createGameObject();
		triangle.model = model;
		triangle.color = { 0.1f, 0.8f, 0.1f };
		triangle.transform2d.translation.x = { 0.2f };
		triangle.transform2d.scale = {2.0f, 0.5f};
		triangle.transform2d.rotation = 0.25f * glm::two_pi<float>();

		gameObjects.push_back(std::move(triangle));
	}

	void EngineApp::createPipelineLayout() {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(this->device.device(), &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void EngineApp::recreateSwapChain() {
		auto extent = this->window.getExtent();
		while(extent.width == 0 || extent.height == 0) {
			extent = this->window.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(this->device.device());

		if (this->swapChain == nullptr) {
			this->swapChain = std::make_unique<EngineSwapChain>(this->device, extent);
		} else {
			this->swapChain = std::make_unique<EngineSwapChain>(this->device, extent, std::move(this->swapChain));
			if (this->swapChain->imageCount() != this->commandBuffers.size()) {
				this->freeCommandBuffers();
				this->createCommandBuffers();
			}
		}

		this->createPipeline();
	}

	void EngineApp::createPipeline() {
		assert(this->swapChain != nullptr && "Cannot create pipeline before swap chain");
		assert(this->pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		EnginePipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = this->swapChain->getRenderPass();
		pipelineConfig.pipelineLayout = this->pipelineLayout;

		this->pipeline = std::make_unique<EnginePipeline>(
			device, 
			"bin/shader/simple_shader.vert.spv",
			"bin/shader/simple_shader.frag.spv",
			pipelineConfig
		);
	}

	void EngineApp::createCommandBuffers() {
		this->commandBuffers.resize(this->swapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = this->device.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(this->commandBuffers.size());

		if (vkAllocateCommandBuffers(this->device.device(), &allocInfo, this->commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffer");
		}
	}

	void EngineApp::freeCommandBuffers() {
		vkFreeCommandBuffers(this->device.device(), this->device.getCommandPool(), static_cast<uint32_t>(this->commandBuffers.size()), this->commandBuffers.data());
		this->commandBuffers.clear();
	}

	void EngineApp::recordCommandBuffer(int imageIndex) {
		VkCommandBufferBeginInfo commandBeginInfo{};
		commandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(this->commandBuffers[imageIndex], &commandBeginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer " + std::to_string(imageIndex));
		}

		VkRenderPassBeginInfo renderBeginInfo{};
		renderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderBeginInfo.renderPass = this->swapChain->getRenderPass();
		renderBeginInfo.framebuffer = this->swapChain->getFrameBuffer(imageIndex);

		renderBeginInfo.renderArea.offset = {0, 0};
		renderBeginInfo.renderArea.extent = this->swapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
		clearValues[1].depthStencil = {1.0f, 0};
		renderBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(this->commandBuffers[imageIndex], &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<uint32_t>(this->swapChain->getSwapChainExtent().width);
		viewport.height = static_cast<uint32_t>(this->swapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{{0, 0}, this->swapChain->getSwapChainExtent()};
		vkCmdSetViewport(this->commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(this->commandBuffers[imageIndex], 0, 1, &scissor);

		this->renderGameObjects(commandBuffers[imageIndex]);

		vkCmdEndRenderPass(this->commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(this->commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer");
		}
	}

	void EngineApp::renderGameObjects(VkCommandBuffer commandBuffer) {
		this->pipeline->bind(commandBuffer);

		for (auto& obj : this->gameObjects) {
			SimplePushConstantData pushConstant{};
			pushConstant.offset = obj.transform2d.translation;
			pushConstant.color = obj.color;
			pushConstant.transform = obj.transform2d.mat2();

			vkCmdPushConstants(
				commandBuffer, 
				pipelineLayout, 
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&pushConstant
			);

			obj.model->bind(commandBuffer);
			obj.model->draw(commandBuffer);
		}
	}
    
	void EngineApp::drawFrame() {
		uint32_t imageIndex;
		auto result = this->swapChain->acquireNextImage(&imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			this->recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image");
		}

		this->recordCommandBuffer(imageIndex);

		result = this->swapChain->submitCommandBuffers(&this->commandBuffers[imageIndex], &imageIndex);
		if (result != VK_ERROR_OUT_OF_DATE_KHR || result != VK_SUBOPTIMAL_KHR || this->window.wasResized()) {
			this->window.resetResizedFlag();
			this->recreateSwapChain();
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image");
		}
	}
}