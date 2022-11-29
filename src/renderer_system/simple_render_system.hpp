#pragma once

#include "../camera/camera.hpp"
#include "../device/device.hpp"
#include "../pipeline/pipeline.hpp"
#include "../game_object/game_object.hpp"
#include "../frame_info.hpp"
#include "../buffer/buffer.hpp"
#include "../descriptor/descriptor.hpp"

#include <memory>
#include <vector>

namespace nugiEngine {
	struct GlobalUBO {
		glm::mat4 projectionView{1.0f};
		glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f};
		glm::vec3 lightPosition{-1.0f};
		alignas(16) glm::vec4 lightColor{-1.0f};
	};

	class EngineSimpleRenderSystem
	{
		public:
			EngineSimpleRenderSystem(EngineDevice& device, VkRenderPass renderPass);
			~EngineSimpleRenderSystem();

			EngineSimpleRenderSystem(const EngineSimpleRenderSystem&) = delete;
			EngineSimpleRenderSystem& operator = (const EngineSimpleRenderSystem&) = delete;

			VkDescriptorSet getGlobalDescriptorSets(int index) { return this->globalDescriptorSets[index]; }

			void writeUniformBuffer(int frameIndex, void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
			void renderGameObjects(VkCommandBuffer commandBuffer, FrameInfo &frameInfo, std::vector<EngineGameObject> &gameObjects);

		private:
			void createPipelineLayout();
			void createPipeline(VkRenderPass renderPass);
			void createBuffers(unsigned long sizeUBO);

			EngineDevice& appDevice;
			
			VkPipelineLayout pipelineLayout;
			std::unique_ptr<EnginePipeline> pipeline;

			std::unique_ptr<EngineDescriptorPool> globalPool{};
			std::unique_ptr<EngineDescriptorSetLayout> globalSetLayout{};
			std::vector<std::shared_ptr<EngineBuffer>> globalUboBuffers;
			std::vector<VkDescriptorSet> globalDescriptorSets;
	};
}