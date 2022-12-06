#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <stdexcept>

namespace nugiEngine {
	class EngineWindow
	{
		public:
			EngineWindow(int w, int h, std::string name);
			~EngineWindow();

			EngineWindow(const EngineWindow&) = delete;
			EngineWindow& operator = (const EngineWindow&) = delete;

			bool shouldClose();
			void pollEvents();

			void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
			VkExtent2D getExtent();
			void resetResizedFlag();
			bool wasResized() { return this->frameBufferResized; }
			GLFWwindow* getWindow() const { return this->window; } 

		private:
			int width;
			int height;
			bool frameBufferResized = false;

			std::string name;
			GLFWwindow *window;

			void init();
			void destroy();
			static void frameBufferResizedCallback(GLFWwindow *window, int width, int height);
	};
}
