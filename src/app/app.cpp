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

		this->renderer = std::make_unique<EngineRayTraceRenderer>(this->window, this->device);
		this->recreateSubRendererAndSubsystem();
	}

	EngineApp::~EngineApp() {}

	void EngineApp::run() {
		auto currentTime = std::chrono::high_resolution_clock::now();
		uint32_t t = 0;

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

			if (this->renderer->acquireFrame()) {
				int imageIndex = this->renderer->getImageIndex();
				int frameIndex = this->renderer->getFrameIndex();

				this->renderer->writeGlobalData(imageIndex);

				auto commandBuffer = this->renderer->beginCommand();
				this->renderer->prepareFrame(commandBuffer);

				auto globalDescSet = this->renderer->getGlobalDescriptorSets(imageIndex);
				this->computeRender->render(commandBuffer, *globalDescSet);

				this->renderer->finishFrame(commandBuffer);
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
		
	}

	void EngineApp::recreateSubRendererAndSubsystem() {
		this->computeRender = std::make_unique<EngineTraceRayRenderSystem>(this->device, 
			this->renderer->getGlobalDescSetLayout()->getDescriptorSetLayout(), *this->renderer->getDescriptorPool(),
			this->renderer->getSwapChain()->getSwapChainExtent().width, this->renderer->getSwapChain()->getSwapChainExtent().height
    );
	}
}