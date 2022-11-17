#include <vulkan/vulkan.hpp>

#include "../window/glfw_window.hpp"

class VulkanRendererSystem
{
protected:
	std::unique_ptr<GlfwAppWindow> window;
	bool IsInitialized();

public:
	VkDevice device = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	VulkanRendererSystem(VkDevice &device, VkPhysicalDevice &physicalDevice);
	~VulkanRendererSystem();

	virtual void InitializeImpl(VkCommandBuffer &commandBuffer, VkRenderPass &renderPass);
	virtual void RenderImpl(VkCommandBuffer &commandBuffer);
	virtual void ShutdownImpl();
};
