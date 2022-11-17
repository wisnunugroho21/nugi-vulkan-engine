#include "renderer/VulkanRenderer.hpp"
#include "renderer/VulkanQuadRenderer.hpp"
#include "window/glfw_window.hpp"

#include <memory>
#include <iostream>
#include <cstdlib>

int main(int argc, char const *argv[])
{
    try {
        std::shared_ptr<GlfwAppWindow> window = std::make_shared<GlfwAppWindow>(800, 600, "Hello Vulkan", nullptr);
        std::shared_ptr<VulkanRenderer> renderer = std::make_shared<VulkanRenderer>(window);
        std::shared_ptr<VulkanRendererSystem> rendererSystem = std::make_shared<VulkanQuadRenderer>(renderer->getLogicalDevice(), renderer->getPhysicalDevice());

        renderer->setRendererSystem(rendererSystem);
        renderer->PrepareRender();

        while (!window->shouldClose())
        {
            window->pollEvents();
            renderer->Render(1);
        }

        renderer->waitIdle();
        renderer->CleanUpRender();
    } catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
