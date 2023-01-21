#include "app.hpp"

#include "../camera/camera.hpp"
#include "../mouse_controller/mouse_controller.hpp"
#include "../keyboard_controller/keyboard_controller.hpp"
#include "../buffer/buffer.hpp"
#include "../frame_info.hpp"
#include "../ray_ubo.hpp"

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
		this->renderer = std::make_unique<EngineRayTraceRenderer>(this->window, this->device);
		this->recreateSubRendererAndSubsystem();
		this->loadObjects();
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
				uint32_t imageIndex = this->renderer->getImageIndex();
				uint32_t frameIndex = this->renderer->getFrameIndex();
				uint32_t randomSeed = this->renderer->getRandomSeed();

				if (!this->traceRayRender->isFrameUpdated[imageIndex]) {
					this->updateCamera(imageIndex);
					this->traceRayRender->isFrameUpdated[imageIndex] = true;
				}

				auto commandBuffer = this->renderer->beginCommand();
				
				this->traceRayRender->prepareFrame(commandBuffer, imageIndex);
				this->traceRayRender->render(commandBuffer, imageIndex, randomSeed);
				this->traceRayRender->finishFrame(commandBuffer, imageIndex);

				std::shared_ptr<VkDescriptorSet> traceRayDescSet = this->traceRayRender->getDescriptorSets(imageIndex);

				this->samplingRayRender->prepareFrame(commandBuffer, imageIndex);
				this->samplingRayRender->render(commandBuffer, imageIndex, traceRayDescSet);
				this->samplingRayRender->finishFrame(commandBuffer, imageIndex);

				this->renderer->endCommand(commandBuffer);
				this->renderer->submitCommand(commandBuffer);

				if (!this->renderer->presentFrame()) {
					this->recreateSubRendererAndSubsystem();
					this->loadObjects();
				}
			}
		}

		vkDeviceWaitIdle(this->device.getLogicalDevice());
	}

	void EngineApp::loadObjects() {
		RayTraceObject objects{};

		objects.spheres[0].radius = 100.0f;
		objects.spheres[0].center = glm::vec3(0.0f, -100.5f, -1.0f);
		objects.lambertians[0].colorAlbedo = glm::vec3(0.8f, 0.8f, 0.0f);
		objects.spheres[0].materialType = 0;
		objects.spheres[0].materialIndex = 0;

		objects.spheres[1].radius = 0.5f;
		objects.spheres[1].center = glm::vec3(0.0f, 0.0f, -1.0f);
		objects.lambertians[1].colorAlbedo = glm::vec3(0.7f, 0.3f, 0.3f);
		objects.spheres[1].materialType = 0;
		objects.spheres[1].materialIndex = 1;

		objects.spheres[2].radius = 0.5f;
		objects.spheres[2].center = glm::vec3(-1.0f, 0.0f, -1.0f);
		objects.dielectrics[0].indexOfRefraction = 1.5f;
		objects.spheres[2].materialType = 2;
		objects.spheres[2].materialIndex = 0;

		objects.spheres[3].radius = 0.5f;
		objects.spheres[3].center = glm::vec3(1.0f, 0.0f, -1.0f);
		objects.metals[0].colorAlbedo = glm::vec3(0.8f, 0.6f, 0.2f);
		objects.metals[0].fuzziness = 0.0f;
		objects.spheres[3].materialType = 1;
		objects.spheres[3].materialIndex = 0;

		uint32_t imageCount = this->renderer->getSwapChain()->getswapChainImages().size();
		for (int i = 0; i < imageCount; i++) {
			this->traceRayRender->writeObjectData(i, objects);
		}
	}

	void EngineApp::updateCamera(uint32_t imageIndex) {
		uint32_t width = this->renderer->getSwapChain()->getSwapChainExtent().width;
		uint32_t height = this->renderer->getSwapChain()->getSwapChainExtent().height;

		RayTraceUbo ubo{};

		glm::vec3 lookFrom = glm::vec3(-2.0f, 2.0f, 1.0f);
		glm::vec3 lookAt = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 vup = glm::vec3(0.0f, 1.0f, 0.0f);
		
		float vfov = 45.0f;
		float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

		float theta = glm::radians(vfov);
		float h = glm::tan(theta / 2.0f);
		float viewportHeight = 2.0f * h;
    float viewportWidth = aspectRatio * viewportHeight;

		glm::vec3 w = glm::normalize(lookFrom - lookAt);
		glm::vec3 u = glm::normalize(glm::cross(vup, w));
    glm::vec3 v = glm::cross(w, u);

		ubo.origin = lookFrom;
		ubo.horizontal = viewportWidth * u;
		ubo.vertical = viewportHeight * v;
		ubo.lowerLeftCorner = ubo.origin - ubo.horizontal / 2.0f + ubo.vertical / 2.0f - w;

		this->traceRayRender->writeGlobalData(imageIndex, ubo);
	}

	void EngineApp::recreateSubRendererAndSubsystem() {
		uint32_t nSample = 8;
		
		uint32_t width = this->renderer->getSwapChain()->getSwapChainExtent().width;
		uint32_t height = this->renderer->getSwapChain()->getSwapChainExtent().height;
		std::shared_ptr<EngineDescriptorPool> descriptorPool = this->renderer->getDescriptorPool();
		std::vector<std::shared_ptr<EngineImage>> swapChainImages = this->renderer->getSwapChain()->getswapChainImages();

		this->traceRayRender = std::make_unique<EngineTraceRayRenderSystem>(this->device, descriptorPool, 
			static_cast<uint32_t>(swapChainImages.size()), width, height, nSample);

		this->samplingRayRender = std::make_unique<EngineSamplingRayRenderSystem>(this->device, descriptorPool, 
			this->traceRayRender->getDescSetLayout(), swapChainImages, width, height);
	}
}