#include "app.hpp"

#include "../renderer_system/simple_render_system.hpp"
#include "../camera/camera.hpp"
#include "../keyboard_controller/keyboard_controller.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <string>
#include <chrono>
#include <iostream>

#define OBJ_POST_X 0.0f
#define OBJ_POST_Y 0.0f
#define OBJ_POST_Z 2.5f

namespace nugiEngine {
	
	

	EngineApp::EngineApp() {
		this->loadObjects();
	}

	EngineApp::~EngineApp() {}

	void EngineApp::run() {
		EngineSimpleRenderSystem renderSystem{this->device, this->renderer.getSwapChainRenderPass()};
		EngineCamera camera{};

		auto viewObject = EngineGameObject::createGameObject();
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
				this->renderer.beginSwapChainRenderPass(commandBuffer);
				renderSystem.renderGameObjects(commandBuffer, this->gameObjects, camera);
				this->renderer.endSwapChainRenderPass(commandBuffer);
				this->renderer.endFrame();
			}
		}

		vkDeviceWaitIdle(this->device.getLogicalDevice());
	}

	void EngineApp::loadObjects() {
		std::shared_ptr<EngineModel> cubeModel = EngineModel::createModelFromFile(this->device, "models/smooth_vase.obj");

		auto gameObj = EngineGameObject::createGameObject();
		gameObj.model = cubeModel;
		gameObj.transform.translation = {OBJ_POST_X, OBJ_POST_Y, OBJ_POST_Z};
		gameObj.transform.scale = glm::vec3{3.0f};
		gameObj.color = {1.0f, 1.0f, 1.0f};

		gameObjects.push_back(std::move(gameObj)); 
	}
}