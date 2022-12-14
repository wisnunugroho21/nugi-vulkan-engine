#include "shadow_map_render_system.hpp"

#include "../swap_chain/swap_chain.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <string>

namespace nugiEngine {

	struct ShadowPushConstantData {
		glm::mat4 modelMatrix{1.0f};
	};

	EngineShadowMapRenderSystem::EngineShadowMapRenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalDescSetLayout) : appDevice{device} {
		this->createPipelineLayout(globalDescSetLayout);
		this->createPipeline(renderPass);
	}

	EngineShadowMapRenderSystem::~EngineShadowMapRenderSystem() {
		vkDestroyPipelineLayout(this->appDevice.getLogicalDevice(), this->pipelineLayout, nullptr);
	}

	void EngineShadowMapRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalDescSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(ShadowPushConstantData);

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

	void EngineShadowMapRenderSystem::createPipeline(VkRenderPass renderPass) {
		assert(this->pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		std::vector<VkVertexInputBindingDescription> bindingDesc { Vertex::getVertexBindingDescriptions()[0] };
		std::vector<VkVertexInputAttributeDescription> attributeDesc { Vertex::getVertexAttributeDescriptions()[0] };

		auto pipelineBuilder = EnginePipeline::Builder(this->appDevice, this->pipelineLayout, renderPass)
			.setDefault("shader/shadow_map_shader.vert.spv", "");

		auto vertexShaderStage = pipelineBuilder.getShaderStagesInfo()[0];

		this->pipeline = pipelineBuilder.setShaderStagesInfo({ vertexShaderStage })
			.setBindingDescriptions(bindingDesc)
			.setAttributeDescriptions(attributeDesc)
			.build();
	}

	void EngineShadowMapRenderSystem::render(std::shared_ptr<EngineCommandBuffer> commandBuffer, VkDescriptorSet UBODescSet, FrameInfo &frameInfo, std::vector<std::shared_ptr<EngineGameObject>> &gameObjects) {
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
			if (obj->textureDescSet != nullptr || obj->light != nullptr) continue;
			
			ShadowPushConstantData pushConstant{};
			pushConstant.modelMatrix = obj->transform.mat4();

			vkCmdPushConstants(
				commandBuffer->getCommandBuffer(), 
				pipelineLayout, 
				VK_SHADER_STAGE_VERTEX_BIT,
				0,
				sizeof(ShadowPushConstantData),
				&pushConstant
			);

			obj->model->bind(commandBuffer);
			obj->model->draw(commandBuffer);
		}
	}
}