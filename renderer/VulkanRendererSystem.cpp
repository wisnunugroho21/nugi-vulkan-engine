#include "VulkanRendererSystem.hpp"

VulkanRendererSystem::VulkanRendererSystem(VkDevice &device, VkPhysicalDevice &physicalDevice) : device{device}, physicalDevice{physicalDevice}
{

}

VulkanRendererSystem::~VulkanRendererSystem()
{
    this->ShutdownImpl();
}