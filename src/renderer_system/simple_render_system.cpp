#include "simple_render_system.hpp"

#include "../swap_chain/swap_chain.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <string>

namespace nugiEngine {

	struct SimplePushConstantData {
		glm::mat4 modelMatrix{1.0f};
		glm::mat4 normalMatrix{1.0f};
	};

	EngineSimpleRenderSystem::EngineSimpleRenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalDescSetLayout) : appDevice{device} {
		this->createPipelineLayout(globalDescSetLayout);
		this->createPipeline(renderPass);
	}

	EngineSimpleRenderSystem::~EngineSimpleRenderSystem() {
		vkDestroyPipelineLayout(this->appDevice.getLogicalDevice(), this->pipelineLayout, nullptr);
	}

	void EngineSimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalDescSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { globalDescSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(this->appDevice.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void EngineSimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
		assert(this->pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		this->pipeline = EnginePipeline::Builder(this->appDevice, this->pipelineLayout, renderPass)
			.setDefault("shader/simple_shader.vert.spv", "shader/simple_shader.frag.spv")
			.build();
	}

	void EngineSimpleRenderSystem::render(std::shared_ptr<EngineCommandBuffer> commandBuffer, VkDescriptorSet &UBODescSet, FrameInfo &frameInfo, std::vector<std::shared_ptr<EngineGameObject>> &gameObjects) {
		this->pipeline->bind(commandBuffer->getCommandBuffer());

		vkCmdBindDescriptorSets(
			commandBuffer->getCommandBuffer(),
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			this->pipelineLayout,
			0,
			1,
			&UBODescSet,
			0,
			nullptr
		);

		for (auto& obj : gameObjects) {
			if (obj->textureDescSet != nullptr || obj->pointLights != nullptr) continue;
			
			SimplePushConstantData pushConstant{};

			pushConstant.modelMatrix = obj->transform.mat4();
			pushConstant.normalMatrix = obj->transform.normalMatrix();

			vkCmdPushConstants(
				commandBuffer->getCommandBuffer(), 
				pipelineLayout, 
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&pushConstant
			);

			obj->model->bind(commandBuffer);
			obj->model->draw(commandBuffer);
		}
	}
}