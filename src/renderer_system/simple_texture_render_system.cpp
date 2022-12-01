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

	EngineSimpleTextureRenderSystem::EngineSimpleTextureRenderSystem(EngineDevice& device, VkRenderPass renderPass, std::vector<const char*> texturesFileName) : appDevice{device} {
		this->createBuffers(sizeof(GlobalUBO));
		this->createTexture(texturesFileName);
		this->createDescriptor();
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

	void EngineSimpleTextureRenderSystem::createTexture(std::vector<const char*> texturesFileName) {
		this->globalTextures.resize(texturesFileName.size());
		for (int i = 0; i < this->globalTextures.size(); i++) {
			this->globalTextures[i] = std::make_unique<EngineTexture>(this->appDevice, texturesFileName[i]);
		}
	}

	void EngineSimpleTextureRenderSystem::createDescriptor() {
		const int totalDescriptorCount = this->globalUboBuffers.size() * this->globalTextures.size();

		this->globalPool = 
			EngineDescriptorPool::Builder(this->appDevice)
				.setMaxSets(totalDescriptorCount)
				.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, totalDescriptorCount)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, totalDescriptorCount)
				.build();

		this->globalSetLayout = 
			EngineDescriptorSetLayout::Builder(this->appDevice)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.build();

		this->globalDescriptorSets.resize(totalDescriptorCount);
		
		for (int iTextures = 0; iTextures < this->globalTextures.size(); iTextures++) {
			for (int iBuffers = 0; iBuffers < this->globalUboBuffers.size(); iBuffers++) {
				const int index = (iTextures * this->globalUboBuffers.size()) + iBuffers;

				auto bufferInfo = this->globalUboBuffers[index]->descriptorInfo();
      	auto textureInfo = this->globalTextures[index]->getDescriptorInfo();

				EngineDescriptorWriter(*this->globalSetLayout, *this->globalPool)
					.writeBuffer(0, &bufferInfo)
					.writeImage(1, &textureInfo)
					.build(this->globalDescriptorSets[index]);
			}
		}
	}

	void EngineSimpleTextureRenderSystem::createPipelineLayout() {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{this->globalSetLayout->getDescriptorSetLayout()}; 

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
		EnginePipeline::defaultPipelineConfigInfo(pipelineConfig);

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

	void EngineSimpleTextureRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, FrameInfo &frameInfo, std::vector<EngineGameObject> &gameObjects) {
		this->pipeline->bind(commandBuffer);

		for (int iObj = 0; iObj < gameObjects.size(); iObj++) {
			auto& obj = gameObjects[iObj];
			auto descpSet = this->getGlobalDescriptorSets(iObj * frameInfo.frameIndex);

			vkCmdBindDescriptorSets(
				commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				this->pipelineLayout,
				0,
				1,
				&descpSet,
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