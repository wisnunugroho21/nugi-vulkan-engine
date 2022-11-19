#include "app.hpp"
#include "../renderer_system/simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <string>

namespace nugiEngine {
	EngineApp::EngineApp() {
		this->loadObjects();
	}

	EngineApp::~EngineApp() {}

	void EngineApp::run() {
		EngineSimpleRenderSystem renderSystem{this->device, this->renderer.getSwapChainRenderPass()};

		while (!this->window.shouldClose()) {
			this->window.pollEvents();

			if (auto commandBuffer = this->renderer.beginFrame()) {
				this->renderer.beginSwapChainRenderPass(commandBuffer);
				renderSystem.renderGameObjects(commandBuffer, this->gameObjects);
				this->renderer.endSwapChainRenderPass(commandBuffer);
				this->renderer.endFrame();
			}
		}

		vkDeviceWaitIdle(this->device.getLogicalDevice());
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
}