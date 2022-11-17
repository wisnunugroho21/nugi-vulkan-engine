#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <stdexcept>

class GlfwAppWindow
{
    public:
        GlfwAppWindow(int w, int h, std::string name, GLFWframebuffersizefun callback);
        ~GlfwAppWindow();

        GlfwAppWindow(const GlfwAppWindow&) = delete;
        GlfwAppWindow& operator = (const GlfwAppWindow&) = delete;

        bool shouldClose();
        void pollEvents();
        void waitEvents();

        const char** getRequiredInstanceExtensions(uint32_t *count);
        VkSurfaceKHR createWindowSurface(VkInstance instance);
        VkExtent2D getExtent();
        void getFramebufferSize(int* width, int* height);

    private:
        const int width;
        const int height;

        std::string name;
        GLFWwindow *window;

        void init(GLFWframebuffersizefun callback);
        void destroy();
};
