#include "sampling_ray_render_system.hpp"

#include "../swap_chain/swap_chain.hpp"
#include "../ray_ubo.hpp"

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
		this->createAccumulateImages(static_cast<uint32_t>(swapChainImages.size()));
		this->createDescriptor(descriptorPool);

		this->createPipelineLayout(traceRayDescLayout);
		this->createPipeline();
	}

	EngineSamplingRayRenderSystem::~EngineSamplingRayRenderSystem() {
		vkDestroyPipelineLayout(this->appDevice.getLogicalDevice(), this->pipelineLayout, nullptr);
	}

	void EngineSamplingRayRenderSystem::createPipelineLayout(std::shared_ptr<EngineDescriptorSetLayout> traceRayDescLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(RayTracePushConstant);

		std::vector<VkDescriptorSetLayout> descSetLayouts = { traceRayDescLayout->getDescriptorSetLayout(), this->descSetLayout->getDescriptorSetLayout() };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

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

	void EngineSamplingRayRenderSystem::createAccumulateImages(uint32_t swapChainImageCount) {
		this->accumulateImages.clear();

		for (uint32_t i = 0; i < swapChainImageCount; i++) {
			auto storageImage = std::make_shared<EngineImage>(
				this->appDevice, this->width, this->height, 
				1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_B8G8R8A8_UNORM, 
				VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT
			);

			this->accumulateImages.emplace_back(storageImage);
		}
	}

	void EngineSamplingRayRenderSystem::createDescriptor(std::shared_ptr<EngineDescriptorPool> descriptorPool) {
		this->descSetLayout = 
			EngineDescriptorSetLayout::Builder(this->appDevice)
				.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
				.build();
				
		this->descriptorSets.clear();
		uint32_t imageCount = static_cast<uint32_t>(this->swapChainImages.size());

		for (uint32_t i = 0; i < imageCount; i++) {
			auto descSet = std::make_shared<VkDescriptorSet>();

			auto accumulateImage = this->accumulateImages[i];
			auto accumulateImageInfo = accumulateImage->getDescriptorInfo(VK_IMAGE_LAYOUT_GENERAL);

			EngineDescriptorWriter(*this->descSetLayout, *descriptorPool)
				.writeImage(0, &accumulateImageInfo)
				.build(descSet.get());

			this->descriptorSets.emplace_back(descSet);
		}
	}

	void EngineSamplingRayRenderSystem::render(std::shared_ptr<EngineCommandBuffer> commandBuffer, uint32_t imageIndex, std::shared_ptr<VkDescriptorSet> traceRayDescSet, uint32_t randomSeed) {
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

		RayTracePushConstant pushConstant{};
		pushConstant.randomSeed = randomSeed;

		vkCmdPushConstants(
			commandBuffer->getCommandBuffer(), 
			this->pipelineLayout, 
			VK_SHADER_STAGE_COMPUTE_BIT,
			0,
			sizeof(RayTracePushConstant),
			&pushConstant
		);

		this->pipeline->dispatch(commandBuffer->getCommandBuffer(), this->width / 8, this->height / 8, 1);
	}

	bool EngineSamplingRayRenderSystem::prepareFrame(std::shared_ptr<EngineCommandBuffer> commandBuffer, uint32_t imageIndex) {
		auto swapChainImage = this->swapChainImages[imageIndex];
		auto accumulateImage = this->accumulateImages[imageIndex];

		if (accumulateImage->getLayout() == VK_IMAGE_LAYOUT_UNDEFINED) {
			accumulateImage->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
				0, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, commandBuffer);
		}

		swapChainImage->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
			0, VK_ACCESS_TRANSFER_WRITE_BIT, commandBuffer);

		return true;
	}

	bool EngineSamplingRayRenderSystem::finishFrame(std::shared_ptr<EngineCommandBuffer> commandBuffer, uint32_t imageIndex) {
		auto swapChainImage = this->swapChainImages[imageIndex];
		auto accumulateImage = this->accumulateImages[imageIndex];

		accumulateImage->transitionImageLayout(VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, commandBuffer);

		swapChainImage->copyImageFromOther(accumulateImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandBuffer);

		swapChainImage->transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, commandBuffer);

		accumulateImage->transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, 
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
			VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, commandBuffer);

    return true;
	}
}