#include "app.hpp"

#include "../camera/camera.hpp"
#include "../mouse_controller/mouse_controller.hpp"
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

		this->renderer = std::make_unique<EngineRenderer>(this->window, this->device);
		this->recreateSubRendererAndSubsystem();
	}

	EngineApp::~EngineApp() {}

	void EngineApp::run() {
		EngineCamera camera{};

		auto viewObject = EngineGameObject::createGameObject();
		viewObject.transform.translation.z = -2.5f;

		EngineMouseController mouseController{};
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
			mouseController.rotateInPlaceXZ(this->window.getWindow(), frameTime, viewObject);

			camera.setViewYXZ(viewObject.transform.translation, viewObject.transform.rotation);

			auto aspect = this->renderer->getSwapChain()->extentAspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 10.0f);

			if (this->renderer->acquireFrame()) {
				int imageIndex = this->renderer->getImageIndex();
				int frameIndex = this->renderer->getFrameIndex();

				FrameInfo frameInfo {
					frameIndex,
					imageIndex,
					frameTime,
					camera
				};

				// update
				GlobalUBO ubo{};
				ubo.projection = camera.getProjectionMatrix();
				ubo.view = camera.getViewMatrix();
				ubo.inverseView = camera.getInverseViewMatrix();
				this->renderer->writeUniformBuffer(frameIndex, &ubo);

				// render
				auto commandBuffer = this->renderer->beginCommand();

				this->rayTracingRenderSystem->render(commandBuffer, *this->renderer->getGlobalDescriptorSets(frameIndex), 
					frameInfo, this->renderer->getSwapChain()->getswapChainImages()[imageIndex]);

				this->renderer->endCommand(commandBuffer);
				this->renderer->submitCommand(commandBuffer);

				if (!this->renderer->presentFrame()) {
					this->recreateSubRendererAndSubsystem();
				}
			}
		}

		vkDeviceWaitIdle(this->device.getLogicalDevice());
	}

	void EngineApp::loadObjects() {
		std::shared_ptr<EngineModel> flatVaseModel = EngineModel::createModelFromFile(this->device, "models/flat_vase.obj");

		auto flatVase = EngineGameObject::createSharedGameObject();
		flatVase->model = flatVaseModel;
		flatVase->transform.translation = {-0.5f, 0.5f, 0.0f};
		flatVase->transform.scale = {3.0f, 1.5f, 3.0f};
		flatVase->color = {1.0f, 1.0f, 1.0f};

		this->gameObjects.push_back(std::move(flatVase)); 

		std::shared_ptr<EngineModel> smoothVaseModel = EngineModel::createModelFromFile(this->device, "models/smooth_vase.obj");

		auto smoothVase = EngineGameObject::createSharedGameObject();
		smoothVase->model = smoothVaseModel;
		smoothVase->transform.translation = {0.5f, 0.5f, 0.0f};
		smoothVase->transform.scale = {3.0f, 1.5f, 3.0f};
		smoothVase->color = {1.0f, 1.0f, 1.0f};

		this->gameObjects.push_back(std::move(smoothVase));
	}

	void EngineApp::recreateSubRendererAndSubsystem() {
		std::vector<EngineTopLevelAccelerationStructure> topAccelStructs;

		for (auto &&obj : this->gameObjects) {
			EngineBottomLevelAccelerationStructure bottomStruct{this->device, deviceProcedure, obj->model};
			EngineTopLevelAccelerationStructure topStruct{this->device, deviceProcedure, bottomStruct};

			topAccelStructs.emplace_back(topStruct);
		}

		this->rayTracingRenderSystem = std::make_unique<EngineRayTracingRenderSystem>(
			this->device, this->deviceProcedure, *this->renderer->getDescriptorPool(), this->renderer->getglobalDescSetLayout()->getDescriptorSetLayout(),
			topAccelStructs, this->renderer->getSwapChain()->imageCount(), this->renderer->getSwapChain()->width(), this->renderer->getSwapChain()->height()
		);
	}
}