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
		uint32_t width, uint32_t height, uint32_t swapChainImageCount) : appDevice{device}, width{width}, height{height} 
	{
		this->createImageStorages(swapChainImageCount);
		this->createUniformBuffer(sizeof(RayTraceUbo), swapChainImageCount);

		this->createDescriptor(descriptorPool, swapChainImageCount);

		this->createPipelineLayout();
		this->createPipeline();
	}

	EngineTraceRayRenderSystem::~EngineTraceRayRenderSystem() {
		vkDestroyPipelineLayout(this->appDevice.getLogicalDevice(), this->pipelineLayout, nullptr);
	}

	void EngineTraceRayRenderSystem::createPipelineLayout() {
		VkDescriptorSetLayout descriptorSetLayout = this->descSetLayout->getDescriptorSetLayout();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

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
		uint32_t nSample = 8;

		for (uint32_t i = 0; i < swapChainImageCount; i++) {
			for (uint32_t j = 0; j < nSample; j++) {
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

	void EngineTraceRayRenderSystem::createUniformBuffer(unsigned long sizeUBO, uint32_t swapChainImageCount) {
		this->uniformBuffers.clear();

		for (uint32_t i = 0; i < swapChainImageCount; i++) {
			auto uniformBuffer = std::make_shared<EngineBuffer>(
				this->appDevice,
				sizeUBO,
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);

			uniformBuffer->map();
			this->uniformBuffers.emplace_back(uniformBuffer);
		}
	}

	void EngineTraceRayRenderSystem::createDescriptor(std::shared_ptr<EngineDescriptorPool> descriptorPool, uint32_t swapChainImageCount) {
		uint32_t nSample = 8;

		this->descSetLayout = 
			EngineDescriptorSetLayout::Builder(this->appDevice)
        .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, nSample)
				.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
				.build();

		for (uint32_t i = 0; i < swapChainImageCount; i++) {
			auto descSet = std::make_shared<VkDescriptorSet>();
			std::vector<VkDescriptorImageInfo> imageInfos{};

			for (uint32_t j = 0; j < nSample; j++) {
				auto imageInfo = this->storageImages[j + nSample * i]->getDescriptorInfo(VK_IMAGE_LAYOUT_GENERAL); 
				imageInfos.emplace_back(imageInfo);
			}

			auto uniformBuffer = this->uniformBuffers[i];
			auto bufferInfo = uniformBuffer->descriptorInfo();

			EngineDescriptorWriter(*this->descSetLayout, *descriptorPool)
				.writeImage(0, imageInfos.data(), nSample) 
				.writeBuffer(1, &bufferInfo)
				.build(descSet.get());

			this->descriptorSets.emplace_back(descSet);
		}
	}

	void EngineTraceRayRenderSystem::writeGlobalData(int imageIndex) {
		RayTraceUbo ubo;

		auto viewport_height = 2.0f;
    auto viewport_width = 1.0f * viewport_height;
    auto focal_length = 1.0f;

		ubo.origin = glm::vec3(0.0f, 0.0f, 0.0f);
    ubo.horizontal = glm::vec3(viewport_width, 0.0f, 0.0f);
    ubo.vertical = glm::vec3(0.0f, viewport_height, 0.0f);
    ubo.lowerLeftCorner = ubo.origin - ubo.horizontal / 2.0f - ubo.vertical / 2.0f - glm::vec3(0.0f, 0.0f, focal_length);

		this->uniformBuffers[imageIndex]->writeToBuffer(&ubo);
		this->uniformBuffers[imageIndex]->flush();
	}

	void EngineTraceRayRenderSystem::render(std::shared_ptr<EngineCommandBuffer> commandBuffer, int imageIndex) {
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

		this->pipeline->dispatch(commandBuffer->getCommandBuffer(), this->width / 8, this->height / 8, 8);
	}

	bool EngineTraceRayRenderSystem::prepareFrame(std::shared_ptr<EngineCommandBuffer> commandBuffer, int imageIndex) {
		uint32_t nSample = 8;

		std::vector<std::shared_ptr<EngineImage>> selectedImages;
		for (uint32_t i = nSample * imageIndex; i < (nSample * imageIndex) + nSample; i++) {
			selectedImages.emplace_back(this->storageImages[i]);
		}

		if (selectedImages[0]->getLayout() == VK_IMAGE_LAYOUT_UNDEFINED) {
			EngineImage::transitionImageLayout(selectedImages, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
				0, VK_ACCESS_SHADER_WRITE_BIT, commandBuffer);
		} else {
			EngineImage::transitionImageLayout(selectedImages, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL, 
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
				VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, commandBuffer);
		}

		return true;
	}

	bool EngineTraceRayRenderSystem::finishFrame(std::shared_ptr<EngineCommandBuffer> commandBuffer, int imageIndex) {
		uint32_t nSample = 8;

		std::vector<std::shared_ptr<EngineImage>> selectedImages;
		for (uint32_t i = nSample * imageIndex; i < (nSample * imageIndex) + nSample; i++) {
			selectedImages.emplace_back(this->storageImages[i]);
		}

		EngineImage::transitionImageLayout(selectedImages, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL, 
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
			VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, 
			commandBuffer);

		return true;
	}
}