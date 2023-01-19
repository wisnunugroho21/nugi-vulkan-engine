#include "sampling_ray_render_system.hpp"

#include "../swap_chain/swap_chain.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <string>

namespace nugiEngine {
	EngineSamplingRayRenderSystem::EngineSamplingRayRenderSystem(EngineDevice& device, std::shared_ptr<EngineDescriptorPool> descriptorPool, std::shared_ptr<EngineDescriptorSetLayout> traceRayDescLayout,
		std::vector<std::shared_ptr<EngineImage>> swapChainImages, uint32_t width, uint32_t height) : appDevice{device}, width{width}, height{height}, swapChainImages{swapChainImages} 
	{
		this->createDescriptor(descriptorPool);

		this->createPipelineLayout(traceRayDescLayout);
		this->createPipeline();
	}

	EngineSamplingRayRenderSystem::~EngineSamplingRayRenderSystem() {
		vkDestroyPipelineLayout(this->appDevice.getLogicalDevice(), this->pipelineLayout, nullptr);
	}

	void EngineSamplingRayRenderSystem::createPipelineLayout(std::shared_ptr<EngineDescriptorSetLayout> traceRayDescLayout) {
		std::vector<VkDescriptorSetLayout> descSetLayouts = { traceRayDescLayout->getDescriptorSetLayout(), this->descSetLayout->getDescriptorSetLayout() };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descSetLayouts.data();

		if (vkCreatePipelineLayout(this->appDevice.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void EngineSamplingRayRenderSystem::createPipeline() {
		assert(this->pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		this->pipeline = EngineComputePipeline::Builder(this->appDevice, this->pipelineLayout)
			.setDefault("shader/ray_trace_sampling.comp.spv")
			.build();
	}

	void EngineSamplingRayRenderSystem::createDescriptor(std::shared_ptr<EngineDescriptorPool> descriptorPool) {
		this->descSetLayout = 
			EngineDescriptorSetLayout::Builder(this->appDevice)
				.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
				.build();
				
		this->descriptorSets.clear();
		uint32_t imageCount = this->swapChainImages.size();

		for (uint32_t i = 0; i < imageCount; i++) {
			auto descSet = std::make_shared<VkDescriptorSet>();

			auto swapChainImage = this->swapChainImages[i];
			auto imageInfo = swapChainImage->getDescriptorInfo(VK_IMAGE_LAYOUT_GENERAL);

			EngineDescriptorWriter(*this->descSetLayout, *descriptorPool)
				.writeImage(0, &imageInfo)
				.build(descSet.get());

			this->descriptorSets.emplace_back(descSet);
		}
	}

	void EngineSamplingRayRenderSystem::render(std::shared_ptr<EngineCommandBuffer> commandBuffer, int imageIndex, std::shared_ptr<VkDescriptorSet> traceRayDescSet) {
		this->pipeline->bind(commandBuffer->getCommandBuffer());

		VkDescriptorSet descpSet[2] = { *traceRayDescSet, *this->descriptorSets[imageIndex] };

		vkCmdBindDescriptorSets(
			commandBuffer->getCommandBuffer(),
			VK_PIPELINE_BIND_POINT_COMPUTE,
			this->pipelineLayout,
			0,
			2,
			descpSet,
			0,
			nullptr
		);

		this->pipeline->dispatch(commandBuffer->getCommandBuffer(), this->width, this->height, 1);
	}

	bool EngineSamplingRayRenderSystem::prepareFrame(std::shared_ptr<EngineCommandBuffer> commandBuffer, int imageIndex) {
		auto swapChainImage = this->swapChainImages[imageIndex];

		swapChainImage->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, commandBuffer);

		return true;
	}

	bool EngineSamplingRayRenderSystem::finishFrame(std::shared_ptr<EngineCommandBuffer> commandBuffer, int imageIndex) {
		auto swapChainImage = this->swapChainImages[imageIndex];

		swapChainImage->transitionImageLayout(VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, commandBuffer);

    return true;
	}
}