#include "trace_ray_render_system.hpp"

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
	EngineTraceRayRenderSystem::EngineTraceRayRenderSystem(EngineDevice& device, std::shared_ptr<EngineDescriptorPool> descriptorPool, 
		uint32_t swapChainImageCount, uint32_t width, uint32_t height, uint32_t nSample) : appDevice{device}, width{width}, height{height}, nSample{nSample} 
	{
		this->createImageStorages(swapChainImageCount);
		this->createUniformBuffer(swapChainImageCount);

		this->createDescriptor(descriptorPool, swapChainImageCount);

		this->createPipelineLayout();
		this->createPipeline();
	}

	EngineTraceRayRenderSystem::~EngineTraceRayRenderSystem() {
		vkDestroyPipelineLayout(this->appDevice.getLogicalDevice(), this->pipelineLayout, nullptr);
	}

	void EngineTraceRayRenderSystem::createPipelineLayout() {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(RayTracePushConstant);

		VkDescriptorSetLayout descriptorSetLayout = this->descSetLayout->getDescriptorSetLayout();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(this->appDevice.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void EngineTraceRayRenderSystem::createPipeline() {
		assert(this->pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		this->pipeline = EngineComputePipeline::Builder(this->appDevice, this->pipelineLayout)
			.setDefault("shader/ray_trace_weekend.comp.spv")
			.build();
	}

	void EngineTraceRayRenderSystem::createImageStorages(uint32_t swapChainImageCount) {
		this->storageImages.clear();

		for (uint32_t i = 0; i < swapChainImageCount; i++) {
			for (uint32_t j = 0; j < this->nSample; j++) {
				auto storageImage = std::make_shared<EngineImage>(
					this->appDevice, this->width, this->height, 
					1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_B8G8R8A8_UNORM, 
					VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT, 
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT
				);

				this->storageImages.emplace_back(storageImage);
			}
		}
	}

	void EngineTraceRayRenderSystem::createUniformBuffer(uint32_t swapChainImageCount) {
		this->uniformBuffers.clear();
		this->objectBuffers.clear();

		for (uint32_t i = 0; i < swapChainImageCount; i++) {
			auto uniformBuffer = std::make_shared<EngineBuffer>(
				this->appDevice,
				sizeof(RayTraceUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);

			uniformBuffer->map();
			this->uniformBuffers.emplace_back(uniformBuffer);
		}

		for (uint32_t i = 0; i < swapChainImageCount; i++) {
			auto objectBuffer = std::make_shared<EngineBuffer>(
				this->appDevice,
				sizeof(RayTraceObject),
				1,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);

			objectBuffer->map();
			this->objectBuffers.emplace_back(objectBuffer);
		}
	}

	void EngineTraceRayRenderSystem::createDescriptor(std::shared_ptr<EngineDescriptorPool> descriptorPool, uint32_t swapChainImageCount) {
		this->descSetLayout = 
			EngineDescriptorSetLayout::Builder(this->appDevice)
        .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, this->nSample)
				.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
				.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
				.build();

		this->descriptorSets.clear();
		this->isFrameUpdated.clear();

		for (uint32_t i = 0; i < swapChainImageCount; i++) {
			auto descSet = std::make_shared<VkDescriptorSet>();
			std::vector<VkDescriptorImageInfo> imageInfos{};

			for (uint32_t j = 0; j < this->nSample; j++) {
				auto imageInfo = this->storageImages[j + this->nSample * i]->getDescriptorInfo(VK_IMAGE_LAYOUT_GENERAL); 
				imageInfos.emplace_back(imageInfo);
			}

			auto uniformBuffer = this->uniformBuffers[i];
			auto uniformBufferInfo = uniformBuffer->descriptorInfo();

			auto objectBuffer = this->objectBuffers[i];
			auto objectBufferInfo = objectBuffer->descriptorInfo();

			EngineDescriptorWriter(*this->descSetLayout, *descriptorPool)
				.writeImage(0, imageInfos.data(), this->nSample) 
				.writeBuffer(1, &uniformBufferInfo)
				.writeBuffer(2, &objectBufferInfo)
				.build(descSet.get());

			this->descriptorSets.emplace_back(descSet);
			this->isFrameUpdated.emplace_back(false);
		}
	}

	void EngineTraceRayRenderSystem::writeGlobalData(uint32_t imageIndex, RayTraceUbo ubo) {
		this->uniformBuffers[imageIndex]->writeToBuffer(&ubo);
		this->uniformBuffers[imageIndex]->flush();
	}

	void EngineTraceRayRenderSystem::writeObjectData(uint32_t imageIndex, RayTraceObject objects) {
		this->objectBuffers[imageIndex]->writeToBuffer(&objects);
		this->objectBuffers[imageIndex]->flush();
	}

	void EngineTraceRayRenderSystem::render(std::shared_ptr<EngineCommandBuffer> commandBuffer, uint32_t imageIndex, uint32_t randomSeed) {
		this->pipeline->bind(commandBuffer->getCommandBuffer());

		vkCmdBindDescriptorSets(
			commandBuffer->getCommandBuffer(),
			VK_PIPELINE_BIND_POINT_COMPUTE,
			this->pipelineLayout,
			0,
			1,
			this->descriptorSets[imageIndex].get(),
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

		this->pipeline->dispatch(commandBuffer->getCommandBuffer(), this->width / 8, this->height / 8, this->nSample / 1);
	}

	bool EngineTraceRayRenderSystem::prepareFrame(std::shared_ptr<EngineCommandBuffer> commandBuffer, uint32_t imageIndex) {
		std::vector<std::shared_ptr<EngineImage>> selectedImages;
		for (uint32_t i = this->nSample * imageIndex; i < (this->nSample * imageIndex) + this->nSample; i++) {
			selectedImages.emplace_back(this->storageImages[i]);
		}

		if (selectedImages[0]->getLayout() == VK_IMAGE_LAYOUT_UNDEFINED) {
			EngineImage::transitionImageLayout(selectedImages, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
				0, VK_ACCESS_SHADER_WRITE_BIT, commandBuffer);
		} else {
			EngineImage::transitionImageLayout(selectedImages, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL, 
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
				VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, commandBuffer);
		}

		return true;
	}

	bool EngineTraceRayRenderSystem::finishFrame(std::shared_ptr<EngineCommandBuffer> commandBuffer, uint32_t imageIndex) {
		std::vector<std::shared_ptr<EngineImage>> selectedImages;
		for (uint32_t i = this->nSample * imageIndex; i < (this->nSample * imageIndex) + this->nSample; i++) {
			selectedImages.emplace_back(this->storageImages[i]);
		}

		EngineImage::transitionImageLayout(selectedImages, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL, 
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
			VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, 
			commandBuffer);

		return true;
	}
}