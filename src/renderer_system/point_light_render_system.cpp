#include "point_light_system.hpp"

#include "../swap_chain/swap_chain.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <string>

namespace nugiEngine {
	EnginePointLightRenderSystem::EnginePointLightRenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalUboDescSetLayout) : appDevice{device} {
		this->createPipelineLayout(globalUboDescSetLayout);
		this->createPipeline(renderPass);
	}

	EnginePointLightRenderSystem::~EnginePointLightRenderSystem() {
		vkDestroyPipelineLayout(this->appDevice.getLogicalDevice(), this->pipelineLayout, nullptr);
	}

	void EnginePointLightRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalUboDescSetLayout) {
		/* VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData); */

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { globalUboDescSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(this->appDevice.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void EnginePointLightRenderSystem::createPipeline(VkRenderPass renderPass) {
		assert(this->pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		EnginePipeline::defaultPipelineConfigInfo(this->appDevice, pipelineConfig);

		pipelineConfig.bindingDescriptions.clear();
		pipelineConfig.attributeDescriptions.clear();

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = this->pipelineLayout;

		this->pipeline = std::make_unique<EnginePipeline>(
			this->appDevice, 
			"shader/point_light.vert.spv",
			"shader/point_light.frag.spv",
			pipelineConfig
		);
	}

	void EnginePointLightRenderSystem::render(VkCommandBuffer commandBuffer, VkDescriptorSet UBODescSet, FrameInfo &frameInfo, std::vector<EngineGameObject> &gameObjects) {
		this->pipeline->bind(commandBuffer);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			this->pipelineLayout,
			0,
			1,
			&UBODescSet,
			0,
			nullptr
		);

		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
	}
}