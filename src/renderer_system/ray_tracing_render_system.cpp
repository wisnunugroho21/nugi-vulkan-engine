#include "ray_tracing_render_system.hpp"

#include "../swap_chain/swap_chain.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <string>

namespace nugiEngine {
	EngineRayTracingRenderSystem::EngineRayTracingRenderSystem(EngineDevice& device, EngineDeviceProcedures &deviceProcedure, EngineDescriptorPool &descriptorPool, 
		VkDescriptorSetLayout globalDescSetLayout, std::vector<EngineTopLevelAccelerationStructure> topLevelAccelStructs, size_t imageCount, uint32_t width, uint32_t height) 
		: appDevice{device}, deviceProcedure{deviceProcedure}, width{width}, height{height}
	{
		this->createStorageImage(imageCount);
		this->createDescriptor(descriptorPool, imageCount, topLevelAccelStructs);
		this->createPipelineLayout(globalDescSetLayout);
		this->createPipeline();
	}

	EngineRayTracingRenderSystem::~EngineRayTracingRenderSystem() {
		vkDestroyPipelineLayout(this->appDevice.getLogicalDevice(), this->pipelineLayout, nullptr);
	}

  void EngineRayTracingRenderSystem::createStorageImage(size_t imageCount) {
    this->storageImages.resize(imageCount);
    for (auto &&image : this->storageImages) {
      image = std::make_shared<EngineImage>(this->appDevice, this->width, this->height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, 
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

      image->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    }
  }

  void EngineRayTracingRenderSystem::createDescriptor(EngineDescriptorPool &descriptorPool, size_t imageCount, std::vector<EngineTopLevelAccelerationStructure> topLevelAccelStructs) {
    this->rayTracingDescSetLayout = 
			EngineDescriptorSetLayout::Builder(this->appDevice)
				.addBinding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_RAYGEN_BIT_KHR)
        .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR)
				.build();

		std::vector<VkAccelerationStructureKHR> accelStructs;
		for (auto &&topStruct : topLevelAccelStructs) {
			accelStructs.emplace_back(topStruct.getAccelStruct());
		}

    this->rayTracingDescSet.resize(imageCount);
		for (int i = 0; i < imageCount; i++) {
			this->rayTracingDescSet[i] = std::make_shared<VkDescriptorSet>();
      
      VkDescriptorImageInfo imageDescriptorInfo{};
      imageDescriptorInfo.imageView   = this->storageImages[i]->getImageView();
      imageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			EngineDescriptorWriter(*this->rayTracingDescSetLayout, descriptorPool)
				.writeAccelStruct(0, accelStructs)
				.writeImage(1, &imageDescriptorInfo)
				.build(this->rayTracingDescSet[i].get());
		}
  }

	void EngineRayTracingRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalDescSetLayout) {
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { globalDescSetLayout, this->rayTracingDescSetLayout->getDescriptorSetLayout() };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

		if (vkCreatePipelineLayout(this->appDevice.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void EngineRayTracingRenderSystem::createPipeline() {
		assert(this->pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		this->pipeline = EngineRayTracingPipeline::Builder(this->appDevice, this->deviceProcedure, this->pipelineLayout)
			.setDefault("shader/simple_raytrace_shader.rgen.spv", "shader/simple_raytrace_shader.rmiss.spv", "shader/simple_raytrace_shader.rchit.spv")
			.build();
	}

	void EngineRayTracingRenderSystem::render(std::shared_ptr<EngineCommandBuffer> commandBuffer, VkDescriptorSet &UBODescSet, FrameInfo &frameInfo, std::shared_ptr<EngineImage> swapChainImage) {
		this->pipeline->bind(commandBuffer->getCommandBuffer());

		VkDescriptorSet descpSet[2] = { UBODescSet, *this->rayTracingDescSet[frameInfo.imageIndex] };

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

		const uint32_t handleSizeAligned = EngineRayTracingPipeline::aligned_size(this->appDevice.getRayTracingProperties().shaderGroupHandleSize, this->appDevice.getRayTracingProperties().shaderGroupHandleAlignment);

		VkStridedDeviceAddressRegionKHR raygenSBTEntry{};
		raygenSBTEntry.deviceAddress = this->pipeline->getRaygenSBTBuffer()->getDeviceAddress();
		raygenSBTEntry.stride = handleSizeAligned;
		raygenSBTEntry.size = handleSizeAligned;

		VkStridedDeviceAddressRegionKHR missSbtEntry{};
		missSbtEntry.deviceAddress = this->pipeline->getMissgenSBTBuffer()->getDeviceAddress();
		missSbtEntry.stride = handleSizeAligned;
		missSbtEntry.size = handleSizeAligned;

		VkStridedDeviceAddressRegionKHR hitSbtEntry{};
		hitSbtEntry.deviceAddress = this->pipeline->getHitgenSBTBuffer()->getDeviceAddress();
		hitSbtEntry.stride = handleSizeAligned;
		hitSbtEntry.size = handleSizeAligned;

		VkStridedDeviceAddressRegionKHR callableSbtEntry{};

		this->deviceProcedure.vkCmdTraceRaysKHR(
			commandBuffer->getCommandBuffer(),
			&raygenSBTEntry,
			&missSbtEntry,
			&hitSbtEntry,
			&callableSbtEntry,
			this->width,
			this->height,
			1
		);

		swapChainImage->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		this->storageImages[frameInfo.imageIndex]->transitionImageLayout(VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		swapChainImage->copy(this->storageImages[frameInfo.imageIndex], commandBuffer);

		swapChainImage->transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		this->storageImages[frameInfo.imageIndex]->transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
	}
}