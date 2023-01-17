#include "compute_render_system.hpp"

#include "../swap_chain/swap_chain.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <string>

namespace nugiEngine {
	EngineRayTracingComputeRender::EngineRayTracingComputeRender(EngineDevice& device, VkDescriptorSetLayout globalDescSetLayout, 
		EngineDescriptorPool &descriptorPool, uint32_t width, uint32_t height) : appDevice{device}, width{width}, height{height} 
	{
		this->createPipelineLayout(globalDescSetLayout);
		this->createPipeline();
	}

	EngineRayTracingComputeRender::~EngineRayTracingComputeRender() {
		vkDestroyPipelineLayout(this->appDevice.getLogicalDevice(), this->pipelineLayout, nullptr);
	}

	void EngineRayTracingComputeRender::createPipelineLayout(VkDescriptorSetLayout globalDescSetLayout) {
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { globalDescSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

		if (vkCreatePipelineLayout(this->appDevice.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void EngineRayTracingComputeRender::createPipeline() {
		assert(this->pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		this->pipeline = EngineComputePipeline::Builder(this->appDevice, this->pipelineLayout)
			.setDefault("shader/simple_ray_trace.comp.spv")
			.build();
	}

	void EngineRayTracingComputeRender::render(std::shared_ptr<EngineCommandBuffer> commandBuffer, VkDescriptorSet &GlobalDescSet) {
		this->pipeline->bind(commandBuffer->getCommandBuffer());

		VkDescriptorSet descpSet[1] = { GlobalDescSet };

		vkCmdBindDescriptorSets(
			commandBuffer->getCommandBuffer(),
			VK_PIPELINE_BIND_POINT_COMPUTE,
			this->pipelineLayout,
			0,
			1,
			descpSet,
			0,
			nullptr
		);

		this->pipeline->dispatch(commandBuffer->getCommandBuffer(), this->width, this->height, 1);
	}
}