#include "app.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <string>

namespace nugiEngine {

	struct SimplePushConstantData {
		glm::mat2 transform{1.0f};
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};
	

	EngineApp::EngineApp() {
		this->loadObjects();
		this->createPipelineLayout();
		this->createPipeline();
	}

	EngineApp::~EngineApp() {
		vkDestroyPipelineLayout(this->device.device(), this->pipelineLayout, nullptr);
	}

	void EngineApp::run() {
		while (!this->window.shouldClose()) {
			this->window.pollEvents();

			if (auto commandBuffer = this->renderer.beginFrame()) {
				this->renderer.beginSwapChainRenderPass(commandBuffer);
				this->renderGameObjects(commandBuffer);
				this->renderer.endSwapChainRenderPass(commandBuffer);
				this->renderer.endFrame();
			}
		}

		vkDeviceWaitIdle(this->device.device());
	}

	void EngineApp::loadObjects() {
		std::vector<Vertex> vertices {
			{{ 0.0f, -0.5f }, {1.0f, 0.0f, 0.0f}},
			{{ 0.5f, 0.5f }, {0.0f, 1.0f, 0.0f}},
			{{ -0.5f, 0.5f }, {0.0f, 0.0f, 1.0f}}
		};

		auto model = std::make_shared<EngineModel>(this->device, vertices); 

		auto triangle = EngineGameObject::createGameObject();
		triangle.model = model;
		triangle.color = { 0.1f, 0.8f, 0.1f };
		triangle.transform2d.translation.x = { 0.2f };
		triangle.transform2d.scale = {2.0f, 0.5f};
		triangle.transform2d.rotation = 0.25f * glm::two_pi<float>();

		gameObjects.push_back(std::move(triangle));
	}

	void EngineApp::createPipelineLayout() {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(this->device.device(), &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void EngineApp::createPipeline() {
		assert(this->pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		EnginePipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = this->renderer.getSwapChainRenderPass();
		pipelineConfig.pipelineLayout = this->pipelineLayout;

		this->pipeline = std::make_unique<EnginePipeline>(
			device, 
			"bin/shader/simple_shader.vert.spv",
			"bin/shader/simple_shader.frag.spv",
			pipelineConfig
		);
	}

	void EngineApp::renderGameObjects(VkCommandBuffer commandBuffer) {
		this->pipeline->bind(commandBuffer);

		for (auto& obj : this->gameObjects) {
			obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.01f, glm::two_pi<float>());

			SimplePushConstantData pushConstant{};
			pushConstant.offset = obj.transform2d.translation;
			pushConstant.color = obj.color;
			pushConstant.transform = obj.transform2d.mat2();

			vkCmdPushConstants(
				commandBuffer, 
				pipelineLayout, 
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