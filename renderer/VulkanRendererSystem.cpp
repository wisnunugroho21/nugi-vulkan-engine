#include "VulkanRendererSystem.hpp"

VulkanRendererSystem::VulkanRendererSystem(const VkDevice &device,const VkPhysicalDevice &physicalDevice) : device{device}, physicalDevice{physicalDevice}
{

}

VulkanRendererSystem::~VulkanRendererSystem()
{
    this->ShutdownImpl();
}

void VulkanRendererSystem::InitializeImpl(VkCommandBuffer &commandBuffer, VkRenderPass &renderPass)
{

}

void VulkanRendererSystem::RenderImpl(VkCommandBuffer &commandBuffer)
{

}

void VulkanRendererSystem::ShutdownImpl()
{

}