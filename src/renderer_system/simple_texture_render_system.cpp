#include "simple_texture_render_system.hpp"

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

	EngineSimpleTextureRenderSystem::EngineSimpleTextureRenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalUboDescSetLayout, size_t objCount) : appDevice{device} {
		this->createDescriptor(objCount);
		this->createPipelineLayout(globalUboDescSetLayout);
		this->createPipeline(renderPass);
	}

	EngineSimpleTextureRenderSystem::~EngineSimpleTextureRenderSystem() {
		vkDestroyPipelineLayout(this->appDevice.getLogicalDevice(), this->pipelineLayout, nullptr);
	}

	void EngineSimpleTextureRenderSystem::createDescriptor(size_t objCount) {
		this->textureDescPool = 
			EngineDescriptorPool::Builder(this->appDevice)
				.setMaxSets(objCount)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, objCount)
				.build();

		this->textureDescSetLayout = 
			EngineDescriptorSetLayout::Builder(this->appDevice)
        .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.build();
	}

	void EngineSimpleTextureRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalUboDescSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { globalUboDescSetLayout, this->textureDescSetLayout->getDescriptorSetLayout() };

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

	void EngineSimpleTextureRenderSystem::createPipeline(VkRenderPass renderPass) {
		assert(this->pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		EnginePipeline::defaultPipelineConfigInfo(this->appDevice, pipelineConfig);

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = this->pipelineLayout;

		this->pipeline = std::make_unique<EnginePipeline>(
			this->appDevice, 
			"shader/simple_texture_shader.vert.spv",
			"shader/simple_texture_shader.frag.spv",
			pipelineConfig
		);
	}

	std::shared_ptr<VkDescriptorSet> EngineSimpleTextureRenderSystem::setupTextureDescriptorSet(VkDescriptorImageInfo descImageInfo) {
		std::shared_ptr<VkDescriptorSet> descSet = std::make_shared<VkDescriptorSet>();

		EngineDescriptorWriter(*this->textureDescSetLayout, *this->textureDescPool)
			.writeImage(0, &descImageInfo)
			.build(descSet.get());

		return descSet;
	}

	void EngineSimpleTextureRenderSystem::render(VkCommandBuffer commandBuffer, VkDescriptorSet UBODescSet, FrameInfo &frameInfo, std::vector<std::shared_ptr<EngineGameObject>> &gameObjects) {
		this->pipeline->bind(commandBuffer);

		for (auto& obj : gameObjects) {
			if (obj->textureDescSet == nullptr) continue;
			
			VkDescriptorSet descpSet[2] = { UBODescSet, *obj->textureDescSet };

			vkCmdBindDescriptorSets(
				commandBuffer,
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
				commandBuffer, 
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