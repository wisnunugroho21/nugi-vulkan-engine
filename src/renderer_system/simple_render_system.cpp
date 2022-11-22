#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <string>

namespace nugiEngine {

	struct SimplePushConstantData {
		glm::mat4 transform{1.0f};
		glm::mat4 modelMatrix{1.0f};
	};

	EngineSimpleRenderSystem::EngineSimpleRenderSystem(EngineDevice& device, VkRenderPass renderPass) : device{device} {
		this->createPipelineLayout();
		this->createPipeline(renderPass);
	}

	EngineSimpleRenderSystem::~EngineSimpleRenderSystem() {
		vkDestroyPipelineLayout(this->device.getLogicalDevice(), this->pipelineLayout, nullptr);
	}

	void EngineSimpleRenderSystem::createPipelineLayout() {
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

		if (vkCreatePipelineLayout(this->device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void EngineSimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
		assert(this->pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		EnginePipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = this->pipelineLayout;

		this->pipeline = std::make_unique<EnginePipeline>(
			device, 
			"bin/shader/simple_shader.vert.spv",
			"bin/shader/simple_shader.frag.spv",
			pipelineConfig
		);
	}

	void EngineSimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<EngineGameObject> &gameObjects, const EngineCamera &camera) {
		this->pipeline->bind(commandBuffer);

		auto projectionView = camera.getProjectionMatrix() * camera.getViewMatrix();

		for (auto& obj : gameObjects) {
			SimplePushConstantData pushConstant{};
			auto modelMatrix = obj.transform.mat4();

			pushConstant.transform = projectionView * modelMatrix;
			pushConstant.modelMatrix = modelMatrix;

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
}