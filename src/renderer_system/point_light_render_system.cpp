#include "point_light_render_system.hpp"

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
	
	EnginePointLightRenderSystem::EnginePointLightRenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalDescSetLayout) : appDevice{device} {
		this->createPipelineLayout(globalDescSetLayout);
		this->createPipeline(renderPass);
	}

	EnginePointLightRenderSystem::~EnginePointLightRenderSystem() {
		vkDestroyPipelineLayout(this->appDevice.getLogicalDevice(), this->pipelineLayout, nullptr);
	}

	void EnginePointLightRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalDescSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PointLightPushConstant);

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

	void EnginePointLightRenderSystem::createPipeline(VkRenderPass renderPass) {
		assert(this->pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		this->pipeline = EnginePipeline::Builder(this->appDevice, this->pipelineLayout, renderPass)
			.setDefault("shader/point_light.vert.spv", "shader/point_light.frag.spv")
			.setBindingDescriptions({})
			.setAttributeDescriptions({})
			.build();
	}

	void EnginePointLightRenderSystem::update(FrameInfo &frameInfo, std::vector<std::shared_ptr<EngineGameObject>> &pointLightObjects, GlobalLight &globalLight) {
		auto rotateLight = glm::rotate(glm::mat4(1.f), 0.5f * frameInfo.frameTime, {0.f, -1.f, 0.f});
		int lightIndex = 0;

		for (auto& plo : pointLightObjects) {
			if (plo->pointLights == nullptr) continue;

			// update light position
     	plo->transform.translation = glm::vec3(rotateLight * glm::vec4(plo->transform.translation, 1.f));

			// copy light to ubo
			globalLight.pointLights[lightIndex].position = glm::vec4{ plo->transform.translation, 1.0f };
			globalLight.pointLights[lightIndex].color = glm::vec4{ plo->color, plo->pointLights->lightIntensity };

			lightIndex++;
		}

		globalLight.numLights = lightIndex;
	}

	void EnginePointLightRenderSystem::render(std::shared_ptr<EngineCommandBuffer> commandBuffer, VkDescriptorSet &UBODescSet, FrameInfo &frameInfo, std::vector<std::shared_ptr<EngineGameObject>> &pointLightObjects) {
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

		for (auto& plo : pointLightObjects) {
			if (plo->pointLights == nullptr) continue;

			PointLightPushConstant pushConstant{};
			pushConstant.position = glm::vec4{ plo->transform.translation, 1.0f };
			pushConstant.color = glm::vec4{ plo->color, plo->pointLights->lightIntensity };
			pushConstant.radius = plo->transform.scale.x;

			vkCmdPushConstants(
				commandBuffer->getCommandBuffer(),
				this->pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PointLightPushConstant),
				&pushConstant
			);

			vkCmdDraw(commandBuffer->getCommandBuffer(), 6, 1, 0, 0);
		}
	}
}