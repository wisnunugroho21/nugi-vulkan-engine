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
	struct PointLightPushConstant {
		glm::vec4 position{};
		glm::vec4 color{};
		float radius;
	};
	
	EnginePointLightRenderSystem::EnginePointLightRenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalUboDescSetLayout) : appDevice{device} {
		this->createPipelineLayout(globalUboDescSetLayout);
		this->createPipeline(renderPass);
	}

	EnginePointLightRenderSystem::~EnginePointLightRenderSystem() {
		vkDestroyPipelineLayout(this->appDevice.getLogicalDevice(), this->pipelineLayout, nullptr);
	}

	void EnginePointLightRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalUboDescSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PointLightPushConstant);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { globalUboDescSetLayout };

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

	void EnginePointLightRenderSystem::update(FrameInfo &frameInfo, std::vector<std::shared_ptr<EngineGameObject>> &pointLightObjects, GlobalUBO &ubo) {
		int lightIndex = 0;
		for (auto& plo : pointLightObjects) {
			if (plo->pointLights == nullptr) continue;

			ubo.pointLights[lightIndex].position = glm::vec4{ plo->transform.translation, 1.0f };
			ubo.pointLights[lightIndex].color = glm::vec4{ plo->color, plo->pointLights->lightIntensity };

			lightIndex++;
		}

		ubo.numLights = lightIndex;

	}

	void EnginePointLightRenderSystem::render(VkCommandBuffer commandBuffer, VkDescriptorSet UBODescSet, FrameInfo &frameInfo, std::vector<std::shared_ptr<EngineGameObject>> &pointLightObjects) {
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

		for (auto& plo : pointLightObjects) {
			if (plo->pointLights == nullptr) continue;

			PointLightPushConstant pushConstant{};
			pushConstant.position = glm::vec4{ plo->transform.translation, 1.0f };
			pushConstant.color = glm::vec4{ plo->color, plo->pointLights->lightIntensity };
			pushConstant.radius = plo->transform.scale.x;

			vkCmdPushConstants(
				commandBuffer,
				this->pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PointLightPushConstant),
				&pushConstant
			);

			vkCmdDraw(commandBuffer, 6, 1, 0, 0);
		}



		
	}
}