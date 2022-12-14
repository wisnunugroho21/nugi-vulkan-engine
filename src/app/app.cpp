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
					frameTime,
					camera
				};

				// update
				GlobalUBO ubo{};
				ubo.projection = camera.getProjectionMatrix();
				ubo.view = camera.getViewMatrix();
				ubo.inverseView = camera.getInverseViewMatrix();
				this->renderer->writeUniformBuffer(frameIndex, &ubo);

				GlobalLight lightingObjects{};
				this->pointLightRenderSystem->update(frameInfo, this->gameObjects, lightingObjects);
				this->renderer->writeLightBuffer(frameIndex, &lightingObjects);

				// render
				auto commandBuffer = this->renderer->beginCommand();
				this->swapChainSubRenderer->beginRenderPass(commandBuffer, imageIndex);

				this->simpleRenderSystem->render(commandBuffer, *this->renderer->getGlobalDescriptorSets(frameIndex), frameInfo, this->gameObjects);
				this->textureRenderSystem->render(commandBuffer, *this->renderer->getGlobalDescriptorSets(frameIndex), frameInfo, this->gameObjects);
				this->pointLightRenderSystem->render(commandBuffer, *this->renderer->getGlobalDescriptorSets(frameIndex), frameInfo, this->gameObjects);
				
				this->swapChainSubRenderer->endRenderPass(commandBuffer);
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

		std::shared_ptr<EngineModel> vikingRoomModel = EngineModel::createModelFromFile(this->device, "models/viking_room.obj");
		std::shared_ptr<EngineTexture> vikingRoomtexture = std::make_shared<EngineTexture>(this->device, "textures/viking_room.png");

		auto vikingRoom = EngineGameObject::createSharedGameObject();
		vikingRoom->model = vikingRoomModel;
		vikingRoom->texture = vikingRoomtexture;
		vikingRoom->transform.translation = {0.0f, -1.0f, -3.0f};
		vikingRoom->transform.scale = {1.0f, 1.0f, 1.0f};
		vikingRoom->color = {1.0f, 1.0f, 1.0f};

		this->gameObjects.push_back(std::move(vikingRoom)); 

		std::shared_ptr<EngineModel> floorModel = EngineModel::createModelFromFile(this->device, "models/quad.obj");

		auto floor = EngineGameObject::createSharedGameObject();
		floor->model = floorModel;
		floor->transform.translation = {0.0f, 0.5f, 0.0f};
		floor->transform.scale = {3.0f, 1.0f, 3.0f};
		floor->color = {1.0f, 1.0f, 1.0f};

		this->gameObjects.push_back(std::move(floor));

		std::vector<glm::vec3> lightColors{
			{1.f, .1f, .1f},
			{.1f, .1f, 1.f},
			{.1f, 1.f, .1f},
			{1.f, 1.f, .1f},
			{.1f, 1.f, 1.f},
			{1.f, 1.f, 1.f}  //
   	};

   	for (int i = 0; i < lightColors.size(); i++) {
			auto rotateLight = glm::rotate(
				glm::mat4(1.f),
				(i * glm::two_pi<float>()) / lightColors.size(),
				{0.f, -1.f, 0.f}
			);

			auto pointLight = EngineGameObject::createSharedPointLight(0.5f, 0.05f);
			pointLight->color = lightColors[i];
			pointLight->transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));

			this->gameObjects.push_back(pointLight);
		}
	}

	void EngineApp::recreateSubRendererAndSubsystem() {
		this->swapChainSubRenderer = std::make_unique<EngineSwapChainSubRenderer>(this->device, this->renderer->getSwapChain()->getswapChainImages(), 
			this->renderer->getSwapChain()->getSwapChainImageFormat(), this->renderer->getSwapChain()->imageCount(), 
			this->renderer->getSwapChain()->width(), this->renderer->getSwapChain()->height());

		this->simpleRenderSystem = std::make_unique<EngineSimpleRenderSystem>(this->device, this->swapChainSubRenderer->getRenderPass()->getRenderPass(), this->renderer->getglobalDescSetLayout()->getDescriptorSetLayout());
		this->pointLightRenderSystem = std::make_unique<EnginePointLightRenderSystem>(this->device, this->swapChainSubRenderer->getRenderPass()->getRenderPass(), this->renderer->getglobalDescSetLayout()->getDescriptorSetLayout());

		std::vector<std::shared_ptr<EngineGameObject>> texturedGameObjects{};
		for (auto& obj : this->gameObjects) {
			if (obj->texture != nullptr) {
				auto& texturedGameObject = obj;
				texturedGameObjects.push_back(texturedGameObject);
			}
		}

		this->textureRenderSystem = std::make_unique<EngineTextureRenderSystem>(this->device, this->swapChainSubRenderer->getRenderPass()->getRenderPass(), this->renderer->getglobalDescSetLayout()->getDescriptorSetLayout());

		for (auto& obj : texturedGameObjects) {
			obj->textureDescSet = this->textureRenderSystem->setupTextureDescriptorSet(*this->renderer->getDescriptorPool(), obj->texture->getDescriptorInfo());
		}
	}
}