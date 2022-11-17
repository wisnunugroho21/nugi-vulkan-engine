#include "glfw_window.hpp"

#include <vector>

GlfwAppWindow::GlfwAppWindow(int w, int h, std::string name, GLFWframebuffersizefun callback) : width{w}, height{h}, name{name} {
	this->init(callback);
}

GlfwAppWindow::~GlfwAppWindow() {
	this->destroy();
}

void GlfwAppWindow::init(GLFWframebuffersizefun callback) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	this->window = glfwCreateWindow(this->width, this->height, this->name.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(this->window, this);
	glfwSetFramebufferSizeCallback(this->window, callback);
}

void GlfwAppWindow::destroy() {
	glfwDestroyWindow(this->window);
	glfwTerminate();
}

bool GlfwAppWindow::shouldClose() {
	return glfwWindowShouldClose(this->window);
}

void GlfwAppWindow::pollEvents() {
	glfwPollEvents();
}

VkSurfaceKHR GlfwAppWindow::createWindowSurface(VkInstance instance) {
	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(instance, this->window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface");
	}

	return surface;
}

VkExtent2D GlfwAppWindow::getExtent() {
	return { static_cast<uint32_t>(this->width), static_cast<uint32_t>(this->height) };
}

void GlfwAppWindow::getFramebufferSize(int* width, int* height) {
	glfwGetFramebufferSize(this->window, width, height);
}

void GlfwAppWindow::waitEvents() {
	glfwWaitEvents();
}

const char** GlfwAppWindow::getRequiredInstanceExtensions(uint32_t *count) {
	return glfwGetRequiredInstanceExtensions(count);
}