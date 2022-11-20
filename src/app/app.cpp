#include "app.hpp"

#include "../renderer_system/simple_render_system.hpp"
#include "../camera/camera.hpp"

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
	
	// temporary helper function, creates a 1x1x1 cube centered at offset
	std::unique_ptr<EngineModel> createCubeModel(EngineDevice& device, glm::vec3 offset) {
		std::vector<Vertex> vertices{
	
			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
	
		};

		for (auto& v : vertices) {
			v.position += offset;
		}

		return std::make_unique<EngineModel>(device, vertices);
	}


	EngineApp::EngineApp() {
		this->loadObjects();
	}

	EngineApp::~EngineApp() {}

	void EngineApp::run() {
		EngineSimpleRenderSystem renderSystem{this->device, this->renderer.getSwapChainRenderPass()};
		EngineCamera camera{};

		auto currentTime = std::chrono::high_resolution_clock::now();
		float timeF = 0.0f;

		while (!this->window.shouldClose()) {
			this->window.pollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			std::cerr << "FPS: " << (1.0f / frameTime) << '\n';

			float camx = 2.0f * glm::sin(glm::radians(timeF)) + OBJ_POST_X;
			float camz = 2.0f * glm::cos(glm::radians(timeF)) + OBJ_POST_Z;

			timeF = timeF + (10.0f * frameTime);

			auto aspect = this->renderer.getAspectRatio();
			camera.setViewTarget(glm::vec3(camx, 1.0f, camz), glm::vec3(OBJ_POST_X, OBJ_POST_Y, OBJ_POST_Z));
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
		std::shared_ptr<EngineModel> cubeModel = createCubeModel(this->device, {0.0f, 0.0f, 0.0f});

		auto cube = EngineGameObject::createGameObject();
		cube.model = cubeModel;
		cube.transform.translation = {OBJ_POST_X, OBJ_POST_Y, OBJ_POST_Z};
		cube.transform.scale = {0.5f, 0.5f, 0.5f};
		cube.color = {1.0f, 1.0f, 1.0f};

		gameObjects.push_back(std::move(cube)); 
	}
}