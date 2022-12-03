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

	EngineSimpleTextureRenderSystem::EngineSimpleTextureRenderSystem(EngineDevice& device, VkRenderPass renderPass, int objCount) : appDevice{device} {
		this->createBuffers(sizeof(GlobalUBO));
		this->createDescriptor(objCount);
		this->createPipelineLayout();
		this->createPipeline(renderPass);
	}

	EngineSimpleTextureRenderSystem::~EngineSimpleTextureRenderSystem() {
		vkDestroyPipelineLayout(this->appDevice.getLogicalDevice(), this->pipelineLayout, nullptr);
	}

	void EngineSimpleTextureRenderSystem::createBuffers(unsigned long sizeUBO) {
		this->globalUboBuffers.resize(EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < this->globalUboBuffers.size(); i++) {
			this->globalUboBuffers[i] = std::make_unique<EngineBuffer>(
				this->appDevice,
				sizeUBO,
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);

			this->globalUboBuffers[i]->map();
		}
	}

	VkDescriptorSet EngineSimpleTextureRenderSystem::setupTextureDescriptorSet(VkDescriptorImageInfo descImageInfo) {
		VkDescriptorSet descSet;
		EngineDescriptorWriter(*this->textureDescSetLayout, *this->textureDescPool)
			.writeImage(0, &descImageInfo)
			.build(descSet);

		return descSet;
	}

	void EngineSimpleTextureRenderSystem::createDescriptor(int objCount) {
		this->bufferDescPool = 
			EngineDescriptorPool::Builder(this->appDevice)
				.setMaxSets(this->globalUboBuffers.size())
				.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, this->globalUboBuffers.size())
				.build();

		this->bufferDescSetLayout = 
			EngineDescriptorSetLayout::Builder(this->appDevice)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.build();

		this->bufferDescriptorSets.resize(this->globalUboBuffers.size());
		
		for (int iBuffers = 0; iBuffers < this->globalUboBuffers.size(); iBuffers++) {
			auto bufferInfo = this->globalUboBuffers[iBuffers]->descriptorInfo();

			EngineDescriptorWriter(*this->bufferDescSetLayout, *this->bufferDescPool)
				.writeBuffer(0, &bufferInfo)
				.build(this->bufferDescriptorSets[iBuffers]);
		}

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

	void EngineSimpleTextureRenderSystem::createPipelineLayout() {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{this->bufferDescSetLayout->getDescriptorSetLayout(), this->textureDescSetLayout->getDescriptorSetLayout()}; 

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
			"bin/shader/simple_texture_shader.vert.spv",
			"bin/shader/simple_texture_shader.frag.spv",
			pipelineConfig
		);
	}

	void EngineSimpleTextureRenderSystem::writeUniformBuffer(int frameIndex, void* data, VkDeviceSize size, VkDeviceSize offset) {
		this->globalUboBuffers[frameIndex]->writeToBuffer(data, size, offset);
		this->globalUboBuffers[frameIndex]->flush(size, offset);
	}

	void EngineSimpleTextureRenderSystem::render(VkCommandBuffer commandBuffer, FrameInfo &frameInfo, std::vector<EngineGameObject> &gameObjects) {
		this->pipeline->bind(commandBuffer);

		for (auto& obj : gameObjects) {
			VkDescriptorSet descpSet[2] = { this->getBufferDescriptorSets(frameInfo.frameIndex), obj.textureDescSet };

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

			pushConstant.modelMatrix = obj.transform.mat4();
			pushConstant.normalMatrix = obj.transform.normalMatrix();

			vkCmdPushConstants(
				commandBuffer, 
				this->pipelineLayout, 
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