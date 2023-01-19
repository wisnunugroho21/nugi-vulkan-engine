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

				auto commandBuffer = this->renderer->beginCommand();

				this->traceRayRender->writeGlobalData(imageIndex);
				this->traceRayRender->render(commandBuffer, imageIndex);
				this->traceRayRender->waitToFinish(commandBuffer, imageIndex);
				std::shared_ptr<VkDescriptorSet> traceRayDescSet = this->traceRayRender->getDescriptorSets(imageIndex);

				this->samplingRayRender->prepareFrame(commandBuffer, imageIndex);
				this->samplingRayRender->render(commandBuffer, imageIndex, traceRayDescSet);
				this->samplingRayRender->finishFrame(commandBuffer, imageIndex);

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
		uint32_t width = this->renderer->getSwapChain()->getSwapChainExtent().width;
		uint32_t height = this->renderer->getSwapChain()->getSwapChainExtent().height;
		std::shared_ptr<EngineDescriptorPool> descriptorPool = this->renderer->getDescriptorPool();
		std::vector<std::shared_ptr<EngineImage>> swapChainImages = this->renderer->getSwapChain()->getswapChainImages();

		this->traceRayRender = std::make_unique<EngineTraceRayRenderSystem>(this->device, descriptorPool,
			width, height, swapChainImages.size());

		this->samplingRayRender = std::make_unique<EngineSamplingRayRenderSystem>(this->device, descriptorPool, 
			this->traceRayRender->getDescSetLayout(), swapChainImages, width, height);
	}
}