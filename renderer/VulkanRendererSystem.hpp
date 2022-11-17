#pragma once

#include <vulkan/vulkan.hpp>

#include "../window/glfw_window.hpp"
#include <memory>

class VulkanRendererSystem
{
protected:
	VkDevice device = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	bool IsInitialized();

public:
	

	int height;
	int width;

	VulkanRendererSystem(const VkDevice &device, const VkPhysicalDevice &physicalDevice);
	~VulkanRendererSystem();

	virtual void InitializeImpl(VkCommandBuffer &commandBuffer, VkRenderPass &renderPass);
	virtual void RenderImpl(VkCommandBuffer &commandBuffer);
	virtual void ShutdownImpl();
};
