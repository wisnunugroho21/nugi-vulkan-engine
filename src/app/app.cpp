#include "app.hpp"

#include "../renderer_system/simple_texture_render_system.hpp"
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
	EngineApp::EngineApp() {
		this->loadObjects();
	}

	EngineApp::~EngineApp() {}

	void EngineApp::run() {
		std::vector<const char*> texturesFilename = {"textures/texture.jpg"};
		
		EngineSimpleTextureRenderSystem renderSystem{this->device, this->renderer.getSwapChainRenderPass(), texturesFilename};
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
					camera,
					renderSystem.getGlobalDescriptorSets(frameIndex)
				};

				// update
				GlobalUBO ubo{};
				ubo.projectionView = camera.getProjectionMatrix() * camera.getViewMatrix();
				renderSystem.writeUniformBuffer(frameIndex, &ubo);

				// render
				this->renderer.beginSwapChainRenderPass(commandBuffer);
				renderSystem.renderGameObjects(commandBuffer, frameInfo, this->gameObjects);
				this->renderer.endSwapChainRenderPass(commandBuffer);
				this->renderer.endFrame(commandBuffer);
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