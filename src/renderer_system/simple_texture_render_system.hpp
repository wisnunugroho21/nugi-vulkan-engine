#pragma once

#include "../camera/camera.hpp"
#include "../device/device.hpp"
#include "../pipeline/pipeline.hpp"
#include "../game_object/game_object.hpp"
#include "../frame_info.hpp"
#include "../buffer/buffer.hpp"
#include "../descriptor/descriptor.hpp"
#include "../texture/texture.hpp"

#include <memory>
#include <vector>

namespace nugiEngine {
	struct GlobalUBO {
		glm::mat4 projectionView{1.0f};
		glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f};
		glm::vec3 lightPosition{-1.0f};
		alignas(16) glm::vec4 lightColor{-1.0f};
	};

	class EngineSimpleTextureRenderSystem
	{
		public:
			EngineSimpleTextureRenderSystem(EngineDevice& device, VkRenderPass renderPass, std::vector<const char*> texturesFileName);
			~EngineSimpleTextureRenderSystem();

			EngineSimpleTextureRenderSystem(const EngineSimpleTextureRenderSystem&) = delete;
			EngineSimpleTextureRenderSystem& operator = (const EngineSimpleTextureRenderSystem&) = delete;

			VkDescriptorSet getGlobalDescriptorSets(int index) { return this->globalDescriptorSets[index]; }

			void writeUniformBuffer(int frameIndex, void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
			void renderGameObjects(VkCommandBuffer commandBuffer, FrameInfo &frameInfo, std::vector<EngineGameObject> &gameObjects);

		private:
			void createPipelineLayout();
			void createPipeline(VkRenderPass renderPass);
			void createBuffers(unsigned long sizeUBO);
			void createTexture(std::vector<const char*> textureFileName);
			void createDescriptor();

			EngineDevice& appDevice;
			
			VkPipelineLayout pipelineLayout;
			std::unique_ptr<EnginePipeline> pipeline;

			std::unique_ptr<EngineDescriptorPool> globalPool{};
			std::unique_ptr<EngineDescriptorSetLayout> globalSetLayout{};
			std::vector<VkDescriptorSet> globalDescriptorSets;

      std::vector<std::shared_ptr<EngineBuffer>> globalUboBuffers;
      std::vector<std::shared_ptr<EngineTexture>> globalTextures;
	};
}