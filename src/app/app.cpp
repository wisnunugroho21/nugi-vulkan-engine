#include "app.hpp"

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
		this->init();
	}

	EngineApp::~EngineApp() {}

	void EngineApp::run() {
		EngineCamera camera{};

		auto viewObject = EngineGameObject::createGameObject();
		viewObject.transform.translation.z = -2.5f;
		EngineKeyboardController keyboardController{};

		auto currentTime = std::chrono::high_resolution_clock::now();
		int t = 0;

		while (!this->window.shouldClose()) {
			this->window.pollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();

			if (t == 1000) {
				std::string appTitle = std::string(APP_TITLE) + std::string(" | FPS: ") + std::to_string((1.0f / frameTime));
				glfwSetWindowTitle(this->window.getWindow(), appTitle.c_str());

				t = 0;
			} else {
				t++;
			}

			currentTime = newTime;

			keyboardController.moveInPlaceXZ(this->window.getWindow(), frameTime, viewObject);
			camera.setViewYXZ(viewObject.transform.translation, viewObject.transform.rotation);

			auto aspect = this->renderer->getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 10.0f);

			if (auto commandBuffer = this->renderer->beginFrame()) {
				int frameIndex = this->renderer->getFrameIndex();
				FrameInfo frameInfo {
					frameIndex,
					frameTime,
					camera
				};

				// update
				GlobalUBO ubo{};
				ubo.projection = camera.getProjectionMatrix();
				ubo.view = camera.getViewMatrix();
				this->renderer->writeUniformBuffer(frameIndex, &ubo);

				// render
				this->renderer->beginSwapChainRenderPass(commandBuffer);

				this->renderSystem->render(commandBuffer, this->renderer->getBufferDescriptorSets(frameIndex), frameInfo, this->gameObjects);
				this->pointLightRenderSystem->render(commandBuffer, this->renderer->getBufferDescriptorSets(frameIndex), frameInfo, this->gameObjects);
				
				this->renderer->endSwapChainRenderPass(commandBuffer);
				this->renderer->endFrame(commandBuffer);
			}
		}

		vkDeviceWaitIdle(this->device.getLogicalDevice());
	}

	void EngineApp::loadObjects() {
		std::shared_ptr<EngineModel> flatVaseModel = EngineModel::createModelFromFile(this->device, "models/flat_vase.obj");
		// std::shared_ptr<EngineTexture> flatVaseTexture = std::make_shared<EngineTexture>(this->device, "texture/flat_vase.obj");

		auto flatVase = EngineGameObject::createGameObject();
		flatVase.model = flatVaseModel;
		// flatVase.texture = flatVaseTexture;
		flatVase.transform.translation = {-0.5f, 0.5f, 0.0f};
		flatVase.transform.scale = {3.0f, 1.5f, 3.0f};
		flatVase.color = {1.0f, 1.0f, 1.0f};

		this->gameObjects.push_back(std::move(flatVase)); 

		std::shared_ptr<EngineModel> smoothVaseModel = EngineModel::createModelFromFile(this->device, "models/smooth_vase.obj");
		// std::shared_ptr<EngineTexture> smoothSaveTexture = std::make_shared<EngineTexture>(this->device, "texture/smooth_vase.obj");

		auto smoothVase = EngineGameObject::createGameObject();
		smoothVase.model = smoothVaseModel;
		// smoothVase.texture = smoothSaveTexture;
		smoothVase.transform.translation = {0.5f, 0.5f, 0.0f};
		smoothVase.transform.scale = {3.0f, 1.5f, 3.0f};
		smoothVase.color = {1.0f, 1.0f, 1.0f};

		this->gameObjects.push_back(std::move(smoothVase)); 

		std::shared_ptr<EngineModel> floorModel = EngineModel::createModelFromFile(this->device, "models/quad.obj");

		auto floor = EngineGameObject::createGameObject();
		floor.model = floorModel;
		floor.transform.translation = {0.0f, 0.5f, 0.0f};
		floor.transform.scale = {3.0f, 1.0f, 3.0f};

		this->gameObjects.push_back(std::move(floor));
	}

	void EngineApp::init() {
		this->renderer = std::make_shared<EngineRenderer>(window, device);
		this->renderSystem = std::make_shared<EngineSimpleRenderSystem>(this->device, this->renderer->getSwapChainRenderPass(), this->renderer->getGlobalUboDescSetLayout());
		this->pointLightRenderSystem = std::make_shared<EnginePointLightRenderSystem>(this->device, this->renderer->getSwapChainRenderPass(), this->renderer->getGlobalUboDescSetLayout());

		// this->renderSystem = std::make_shared<EngineSimpleTextureRenderSystem>(this->device, this->renderer->getSwapChainRenderPass(), this->renderer->getGlobalUboDescSetLayout(), this->gameObjects.size());

		/* for (auto& obj : this->gameObjects) {
			obj.textureDescSet = this->renderSystem->setupTextureDescriptorSet(obj.texture->getDescriptorInfo());
		} */
	}
}