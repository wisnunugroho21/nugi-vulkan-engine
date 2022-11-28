#include "app.hpp"

#include "../renderer_system/simple_render_system.hpp"
#include "../camera/camera.hpp"
#include "../keyboard_controller/keyboard_controller.hpp"
#include "../buffer/buffer.hpp"
#include "../frame_info.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <string>
#include <chrono>
#include <iostream>

namespace nugiEngine {
	struct GlobalUBO {
		glm::mat4 projectionView{1.0f};
		glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f};
		glm::vec3 lightPosition{-1.0f};
		alignas(16) glm::vec4 lightColor{-1.0f};
	};

	EngineApp::EngineApp() {
		this->globalPool = 
			EngineDescriptorPool::Builder(this->device)
				.setMaxSets(EngineSwapChain::MAX_FRAMES_IN_FLIGHT)
				.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, EngineSwapChain::MAX_FRAMES_IN_FLIGHT)
				.build();

		this->loadObjects();
	}

	EngineApp::~EngineApp() {}

	void EngineApp::run() {
		std::vector<std::unique_ptr<EngineBuffer>> globalUboBuffers(EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalUboBuffers.size(); i++) {
			globalUboBuffers[i] = std::make_unique<EngineBuffer>(
				this->device,
				sizeof(GlobalUBO),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);

			globalUboBuffers[i]->map();
		}

		auto globalSetLayout = 
			EngineDescriptorSetLayout::Builder(this->device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = globalUboBuffers[i]->descriptorInfo();
			EngineDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		EngineSimpleRenderSystem renderSystem{this->device, this->renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
		EngineCamera camera{};

		auto viewObject = EngineGameObject::createGameObject();
		viewObject.transform.translation.z = -2.5f;
		EngineKeyboardController keyboardController{};

		auto currentTime = std::chrono::high_resolution_clock::now();
		while (!this->window.shouldClose()) {
			this->window.pollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			std::cerr << "FPS: " << (1.0f / frameTime) << '\n';

			keyboardController.moveInPlaceXZ(this->window.getWindow(), frameTime, viewObject);
			camera.setViewYXZ(viewObject.transform.translation, viewObject.transform.rotation);

			auto aspect = this->renderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 10.0f);

			if (auto commandBuffer = this->renderer.beginFrame()) {
				int frameIndex = this->renderer.getFrameIndex();
				FrameInfo frameInfo {
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex]
				};

				// update
				GlobalUBO ubo{};
				ubo.projectionView = camera.getProjectionMatrix() * camera.getViewMatrix();
				globalUboBuffers[frameIndex]->writeToBuffer(&ubo);
				globalUboBuffers[frameIndex]->flush();

				// render
				this->renderer.beginSwapChainRenderPass(frameInfo.commandBuffer);
				renderSystem.renderGameObjects(frameInfo, this->gameObjects);
				this->renderer.endSwapChainRenderPass(frameInfo.commandBuffer);
				this->renderer.endFrame();
			}
		}

		vkDeviceWaitIdle(this->device.getLogicalDevice());
	}

	void EngineApp::loadObjects() {
		std::shared_ptr<EngineModel> flatVaseModel = EngineModel::createModelFromFile(this->device, "models/flat_vase.obj");

		auto flatVase = EngineGameObject::createGameObject();
		flatVase.model = flatVaseModel;
		flatVase.transform.translation = {-0.5f, 0.5f, 0.0f};
		flatVase.transform.scale = {3.0f, 1.5f, 3.0f};
		flatVase.color = {1.0f, 1.0f, 1.0f};

		this->gameObjects.push_back(std::move(flatVase)); 

		std::shared_ptr<EngineModel> smoothVaseModel = EngineModel::createModelFromFile(this->device, "models/smooth_vase.obj");

		auto smoothVase = EngineGameObject::createGameObject();
		smoothVase.model = smoothVaseModel;
		smoothVase.transform.translation = {0.5f, 0.5f, 0.0f};
		smoothVase.transform.scale = {3.0f, 1.5f, 3.0f};
		smoothVase.color = {1.0f, 1.0f, 1.0f};

		this->gameObjects.push_back(std::move(smoothVase)); 
	}
}