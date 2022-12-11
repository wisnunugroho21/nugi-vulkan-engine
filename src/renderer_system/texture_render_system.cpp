#include "texture_render_system.hpp"

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

	EngineTextureRenderSystem::EngineTextureRenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalDescSetLayout) 
		: appDevice{device} 
	{
		this->createDescriptor();
		this->createPipelineLayout(globalDescSetLayout);
		this->createPipeline(renderPass);
	}

	EngineTextureRenderSystem::~EngineTextureRenderSystem() {
		vkDestroyPipelineLayout(this->appDevice.getLogicalDevice(), this->pipelineLayout, nullptr);
	}

	void EngineTextureRenderSystem::createDescriptor() {
		this->textureDescSetLayout = 
			EngineDescriptorSetLayout::Builder(this->appDevice)
        .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.build();
	}

	void EngineTextureRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalDescSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { globalDescSetLayout, this->textureDescSetLayout->getDescriptorSetLayout() };

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

	void EngineTextureRenderSystem::createPipeline(VkRenderPass renderPass) {
		assert(this->pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		this->pipeline = EnginePipeline::Builder(this->appDevice, this->pipelineLayout, renderPass)
			.setDefault("shader/simple_texture_shader.vert.spv", "shader/simple_texture_shader.frag.spv")
			.build();
	}

	std::shared_ptr<VkDescriptorSet> EngineTextureRenderSystem::setupTextureDescriptorSet(EngineDescriptorPool &descriptorPool, VkDescriptorImageInfo descImageInfo) {
		std::shared_ptr<VkDescriptorSet> descSet = std::make_shared<VkDescriptorSet>();

		EngineDescriptorWriter(*this->textureDescSetLayout, descriptorPool)
			.writeImage(0, &descImageInfo)
			.build(descSet.get());

		return descSet;
	}

	void EngineTextureRenderSystem::render(std::shared_ptr<EngineCommandBuffer> commandBuffer, VkDescriptorSet &UBODescSet, FrameInfo &frameInfo, std::vector<std::shared_ptr<EngineGameObject>> &gameObjects) {
		this->pipeline->bind(commandBuffer->getCommandBuffer());

		for (auto& obj : gameObjects) {
			if (obj->textureDescSet == nullptr) continue;
			
			VkDescriptorSet descpSet[2] = { UBODescSet, *obj->textureDescSet };

			vkCmdBindDescriptorSets(
				commandBuffer->getCommandBuffer(),
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				this->pipelineLayout,
				0,
				2,
				descpSet,
				0,
				nullptr
			);

			SimplePushConstantData pushConstant{};

			pushConstant.modelMatrix = obj->transform.mat4();
			pushConstant.normalMatrix = obj->transform.normalMatrix();

			vkCmdPushConstants(
				commandBuffer->getCommandBuffer(), 
				this->pipelineLayout, 
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