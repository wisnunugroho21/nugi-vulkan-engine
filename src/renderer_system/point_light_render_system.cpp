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
	EnginePointLightRenderSystem::EnginePointLightRenderSystem(EngineDevice& device, VkRenderPass renderPass) : appDevice{device} {
		this->createBuffers(sizeof(GlobalUBO));
		this->createDescriptor();
		this->createPipelineLayout();
		this->createPipeline(renderPass);
	}

	EnginePointLightRenderSystem::~EnginePointLightRenderSystem() {
		vkDestroyPipelineLayout(this->appDevice.getLogicalDevice(), this->pipelineLayout, nullptr);
	}

	void EnginePointLightRenderSystem::createBuffers(unsigned long sizeUBO) {
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

	void EnginePointLightRenderSystem::createDescriptor() {
		this->globalPool = 
			EngineDescriptorPool::Builder(this->appDevice)
				.setMaxSets(EngineSwapChain::MAX_FRAMES_IN_FLIGHT)
				.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, EngineSwapChain::MAX_FRAMES_IN_FLIGHT)
				.build();

		this->globalSetLayout = 
			EngineDescriptorSetLayout::Builder(this->appDevice)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.build();

		this->globalDescriptorSets.resize(EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < this->globalDescriptorSets.size(); i++) {
			auto bufferInfo = this->globalUboBuffers[i]->descriptorInfo();
			EngineDescriptorWriter(*this->globalSetLayout, *this->globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(this->globalDescriptorSets[i]);
		}
	}

	void EnginePointLightRenderSystem::createPipelineLayout() {
		/* VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData); */

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{this->globalSetLayout->getDescriptorSetLayout()}; 

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

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = this->pipelineLayout;

		this->pipeline = std::make_unique<EnginePipeline>(
			this->appDevice, 
			"bin/shader/point_light.vert.spv",
			"bin/shader/point_light.frag.spv",
			pipelineConfig
		);
	}

	void EnginePointLightRenderSystem::writeUniformBuffer(int frameIndex, void* data, VkDeviceSize size, VkDeviceSize offset) {
		this->globalUboBuffers[frameIndex]->writeToBuffer(data, size, offset);
		this->globalUboBuffers[frameIndex]->flush(size, offset);
	}

	void EnginePointLightRenderSystem::render(VkCommandBuffer commandBuffer, FrameInfo &frameInfo, std::vector<EngineGameObject> &gameObjects) {
		this->pipeline->bind(commandBuffer);
		auto globalDescSet = this->getGlobalDescriptorSets(frameInfo.frameIndex);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			this->pipelineLayout,
			0,
			1,
			&globalDescSet,
			0,
			nullptr
		);

		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
	}
}