//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "VulkanRenderer.hpp"

#include <iostream>
#include <algorithm>

#include "Utility.hpp"

#include <cassert>
#include <iostream>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#pragma warning( disable : 4100 ) // disable unreferenced formal parameter warnings

struct ImportTable
{
#define GET_INSTANCE_ENTRYPOINT(i, w) w = reinterpret_cast<PFN_##w>(vkGetInstanceProcAddr(i, #w))
#define GET_DEVICE_ENTRYPOINT(i, w) w = reinterpret_cast<PFN_##w>(vkGetDeviceProcAddr(i, #w))

	ImportTable() = default;

	ImportTable(VkInstance instance, VkDevice device)
	{
#ifdef _DEBUG
		GET_INSTANCE_ENTRYPOINT(instance, vkCreateDebugReportCallbackEXT);
		GET_INSTANCE_ENTRYPOINT(instance, vkDebugReportMessageEXT);
		GET_INSTANCE_ENTRYPOINT(instance, vkDestroyDebugReportCallbackEXT);
#endif
	}

#undef GET_INSTANCE_ENTRYPOINT
#undef GET_DEVICE_ENTRYPOINT

#ifdef _DEBUG
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = nullptr;
	PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT = nullptr;
	PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = nullptr;
#endif
};

///////////////////////////////////////////////////////////////////////////////
VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(
	VkDebugReportFlagsEXT       /*flags*/,
	VkDebugReportObjectTypeEXT  /*objectType*/,
	uint64_t                    /*object*/,
	size_t                      /*location*/,
	int32_t                     /*messageCode*/,
	const char*                 /*pLayerPrefix*/,
	const char*                 pMessage,
	void*                       /*pUserData*/)
{
	std::cerr << pMessage << '\n';
	return VK_FALSE;
}

///////////////////////////////////////////////////////////////////////////////
std::vector<const char*> VulkanRenderer::GetDebugInstanceLayerNames()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> instanceLayers{ layerCount };
	vkEnumerateInstanceLayerProperties(&layerCount, instanceLayers.data());

	std::vector<const char*> result;
	for (const auto& p : instanceLayers)
	{
		if (strcmp(p.layerName, "VK_LAYER_LUNARG_standard_validation") == 0)
		{
			result.push_back("VK_LAYER_LUNARG_standard_validation");
		}
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////////
std::vector<const char*> VulkanRenderer::GetDebugInstanceExtensionNames()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> instanceExtensions{ extensionCount };
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, instanceExtensions.data());

	std::vector<const char*> result;
	for (const auto& e : instanceExtensions)
	{
		if (strcmp(e.extensionName, "VK_EXT_debug_report") == 0)
		{
			result.push_back("VK_EXT_debug_report");
		}
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////////
std::vector<const char*> VulkanRenderer::GetDebugDeviceLayerNames(VkPhysicalDevice device)
{
	uint32_t layerCount = 0;
	vkEnumerateDeviceLayerProperties(device, &layerCount, nullptr);

	std::vector<VkLayerProperties> deviceLayers{ layerCount };
	vkEnumerateDeviceLayerProperties(device, &layerCount, deviceLayers.data());

	std::vector<const char*> result;
	for (const auto& p : deviceLayers)
	{
		if (strcmp(p.layerName, "VK_LAYER_LUNARG_standard_validation") == 0)
		{
			result.push_back("VK_LAYER_LUNARG_standard_validation");
		}
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////////
void VulkanRenderer::FindPhysicalDeviceWithGraphicsQueue(const std::vector<VkPhysicalDevice>& physicalDevices,
  VkPhysicalDevice* outputDevice, int* outputGraphicsQueueIndex)
{
    for (auto physicalDevice : physicalDevices)
    {
        uint32_t queueFamilyPropertyCount = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
            &queueFamilyPropertyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilyProperties{ queueFamilyPropertyCount };
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
            &queueFamilyPropertyCount, queueFamilyProperties.data());

        int i = 0;
        for (const auto& queueFamilyProperty : queueFamilyProperties)
        {
            if (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                if (outputDevice)
                {
                    *outputDevice = physicalDevice;
                }

                if (outputGraphicsQueueIndex)
                {
                    *outputGraphicsQueueIndex = i;
                }

                return;
            }

            ++i;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
VkInstance VulkanRenderer::CreateInstance()
{
    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    std::vector<const char*> instanceExtensions =
    {
        "VK_KHR_surface", "VK_KHR_win32_surface"
    };

#ifdef _DEBUG
    auto debugInstanceExtensionNames = GetDebugInstanceExtensionNames();
    instanceExtensions.insert(instanceExtensions.end(),
        debugInstanceExtensionNames.begin(), debugInstanceExtensionNames.end());
#endif

    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t> (instanceExtensions.size());

    std::vector<const char*> instanceLayers;

#ifdef _DEBUG
    auto debugInstanceLayerNames = GetDebugInstanceLayerNames();
    instanceLayers.insert(instanceLayers.end(),
        debugInstanceLayerNames.begin(), debugInstanceLayerNames.end());
#endif

    instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t> (instanceLayers.size());

    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.apiVersion = VK_API_VERSION_1_0;
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName = "AMD Vulkan Sample application";
    applicationInfo.pEngineName = "AMD Vulkan Sample Engine";

    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    VkInstance instance = VK_NULL_HANDLE;
    vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

    return instance;
}

///////////////////////////////////////////////////////////////////////////////
void VulkanRenderer::CreateDeviceAndQueue(VkInstance instance, VkDevice* outputDevice,
    VkQueue* outputQueue, int* outputQueueIndex,
    VkPhysicalDevice* outputPhysicalDevice)
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

    std::vector<VkPhysicalDevice> devices{ physicalDeviceCount };
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount,
        devices.data());

    VkPhysicalDevice physicalDevice = nullptr;
    int graphicsQueueIndex = -1;

    FindPhysicalDeviceWithGraphicsQueue(devices, &physicalDevice, &graphicsQueueIndex);

    assert(physicalDevice);

    VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.queueFamilyIndex = graphicsQueueIndex;

    static const float queuePriorities[] = { 1.0f };
    deviceQueueCreateInfo.pQueuePriorities = queuePriorities;

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;

    std::vector<const char*> deviceLayers;

#ifdef _DEBUG
    auto debugDeviceLayerNames = GetDebugDeviceLayerNames(physicalDevice);
    deviceLayers.insert(deviceLayers.end(),
        debugDeviceLayerNames.begin(), debugDeviceLayerNames.end());
#endif

    deviceCreateInfo.ppEnabledLayerNames = deviceLayers.data();
    deviceCreateInfo.enabledLayerCount = static_cast<uint32_t> (deviceLayers.size());

	// Our SPIR-V requests this, so we need to enable it here
	VkPhysicalDeviceFeatures enabledFeatures = {};
	enabledFeatures.shaderClipDistance = true;

	deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

    std::vector<const char*> deviceExtensions =
    {
        "VK_KHR_swapchain"
    };

    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t> (deviceExtensions.size());

    VkDevice device = nullptr;
    vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
    assert(device);

    VkQueue queue = nullptr;
    vkGetDeviceQueue(device, graphicsQueueIndex, 0, &queue);
    assert(queue);

    if (outputQueue)
    {
        *outputQueue = queue;
    }

    if (outputDevice)
    {
        *outputDevice = device;
    }

    if (outputQueueIndex)
    {
        *outputQueueIndex = graphicsQueueIndex;
    }

    if (outputPhysicalDevice)
    {
        *outputPhysicalDevice = physicalDevice;
    }
}

///////////////////////////////////////////////////////////////////////////////
SwapchainFormatColorSpace VulkanRenderer::GetSwapchainFormatAndColorspace(VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface, ImportTable* importTable)
{
    uint32_t surfaceFormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,
        surface, &surfaceFormatCount, nullptr);

    std::vector<VkSurfaceFormatKHR> surfaceFormats{ surfaceFormatCount };
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,
        surface, &surfaceFormatCount, surfaceFormats.data());

    SwapchainFormatColorSpace result;

    if (surfaceFormatCount == 1 && surfaceFormats.front().format == VK_FORMAT_UNDEFINED)
    {
        result.format = VK_FORMAT_R8G8B8A8_UNORM;
    }
    else
    {
        result.format = surfaceFormats.front().format;
    }

    result.colorSpace = surfaceFormats.front().colorSpace;

    return result;
}

///////////////////////////////////////////////////////////////////////////////
VkRenderPass VulkanRenderer::CreateRenderPass(VkDevice device, VkFormat swapchainFormat)
{
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.format = swapchainFormat;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentReference attachmentReference = {};
    attachmentReference.attachment = 0;
    attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pColorAttachments = &attachmentReference;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.pAttachments = &attachmentDescription;

    VkRenderPass result = nullptr;
    vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &result);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
void VulkanRenderer::CreateFramebuffers(VkDevice device, VkRenderPass renderPass,
    const int width, const int height,
    const int count, const VkImageView* imageViews, VkFramebuffer* framebuffers)
{
    for (int i = 0; i < count; ++i)
    {
        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = &imageViews[i];
        framebufferCreateInfo.height = height;
        framebufferCreateInfo.width = width;
        framebufferCreateInfo.layers = 1;
        framebufferCreateInfo.renderPass = renderPass;

        vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr,
            &framebuffers[i]);
    }
}

///////////////////////////////////////////////////////////////////////////////
void VulkanRenderer::CreateSwapchainImageViews(VkDevice device, VkFormat format,
    const int count, const VkImage* images, VkImageView* imageViews)
{
    for (int i = 0; i < count; ++i)
    {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = images[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = format;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        vkCreateImageView(device, &imageViewCreateInfo, nullptr,
            &imageViews[i]);
    }
}

///////////////////////////////////////////////////////////////////////////////
VkSwapchainKHR VulkanRenderer::CreateSwapchain(VkPhysicalDevice physicalDevice, VkDevice device,
    VkSurfaceKHR surface, const int surfaceWidth, const int surfaceHeight,
    const int backbufferCount, ImportTable* importTable,
    VkFormat* swapchainFormat)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice,
        surface, &surfaceCapabilities);

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,
        surface, &presentModeCount, nullptr);

    std::vector<VkPresentModeKHR> presentModes{ presentModeCount };

    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,
        surface, &presentModeCount, presentModes.data());

    VkExtent2D swapChainSize = {};
    swapChainSize = surfaceCapabilities.currentExtent;
    assert(static_cast<int> (swapChainSize.width) == surfaceWidth);
    assert(static_cast<int> (swapChainSize.height) == surfaceHeight);

    uint32_t swapChainImageCount = backbufferCount;
    assert(swapChainImageCount >= surfaceCapabilities.minImageCount);

    // 0 indicates unlimited number of images
    if (surfaceCapabilities.maxImageCount != 0)
    {
        assert(swapChainImageCount < surfaceCapabilities.maxImageCount);
    }

    VkSurfaceTransformFlagBitsKHR surfaceTransformFlags;

    if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        surfaceTransformFlags = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        surfaceTransformFlags = surfaceCapabilities.currentTransform;
    }

    auto swapchainFormatColorSpace = GetSwapchainFormatAndColorspace(physicalDevice, surface,
        importTable);

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = swapChainImageCount;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.preTransform = surfaceTransformFlags;
    swapchainCreateInfo.imageColorSpace = swapchainFormatColorSpace.colorSpace;
    swapchainCreateInfo.imageFormat = swapchainFormatColorSpace.format;
    swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.imageExtent = swapChainSize;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;

    VkSwapchainKHR swapchain;
    vkCreateSwapchainKHR(device, &swapchainCreateInfo,
        nullptr, &swapchain);

    if (swapchainFormat)
    {
        *swapchainFormat = swapchainFormatColorSpace.format;
    }

    return swapchain;
}

///////////////////////////////////////////////////////////////////////////////
VkSurfaceKHR VulkanRenderer::CreateSurface(VkInstance instance, GlfwAppWindow *window)
{
  return window->createWindowSurface(instance);
}

#ifdef _DEBUG
///////////////////////////////////////////////////////////////////////////////
VkDebugReportCallbackEXT SetupDebugCallback(VkInstance instance, VulkanSample::ImportTable* importTable)
{
    if (importTable->vkCreateDebugReportCallbackEXT)
    {
        VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
        callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
        callbackCreateInfo.flags =
            VK_DEBUG_REPORT_ERROR_BIT_EXT |
            VK_DEBUG_REPORT_WARNING_BIT_EXT |
            VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        callbackCreateInfo.pfnCallback = &DebugReportCallback;

        VkDebugReportCallbackEXT callback;
        importTable->vkCreateDebugReportCallbackEXT(instance, &callbackCreateInfo, nullptr, &callback);
        return callback;
    }
    else
    {
        return VK_NULL_HANDLE;
    }
}

///////////////////////////////////////////////////////////////////////////////
void CleanupDebugCallback(VkInstance instance, VkDebugReportCallbackEXT callback,
    VulkanSample::ImportTable* importTable)
{
    if (importTable->vkDestroyDebugReportCallbackEXT)
    {
        importTable->vkDestroyDebugReportCallbackEXT(instance, callback, nullptr);
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////
VulkanRenderer::VulkanRenderer(std::shared_ptr<GlfwAppWindow> window)
{
	this->instance = VulkanRenderer::CreateInstance();
	if (this->instance == VK_NULL_HANDLE) // just bail out if the user does not have a compatible Vulkan driver
	{
		return;
	}

	VkPhysicalDevice physicalDevice;
	VulkanRenderer::CreateDeviceAndQueue(this->instance, &this->device, &this->queue, &this->queueFamilyIndex, &this->physicalDevice);
	this->physicalDevice = physicalDevice;

	this->importTable.reset(new ImportTable{this->instance, this->device});

#ifdef _DEBUG
  this->debugCallback = SetupDebugCallback(instance_, importTable_.get());
#endif

	this->window = window;
	this->surface = VulkanRenderer::CreateSurface(this->instance, this->window.get());

	VkBool32 presentSupported;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 0, this->surface, &presentSupported);
	assert(presentSupported);

	VkFormat swapchainFormat = VK_FORMAT_UNDEFINED;
	this->swapchain = VulkanRenderer::CreateSwapchain(physicalDevice, this->device, this->surface, this->window->getExtent().width, 
		this->window->getExtent().height, QUEUE_SLOT_COUNT, this->importTable.get(), &swapchainFormat);

	assert(this->swapchain);

	uint32_t swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(this->device, this->swapchain, &swapchainImageCount, nullptr);
	assert(static_cast<int> (swapchainImageCount) == QUEUE_SLOT_COUNT);

	vkGetSwapchainImagesKHR(this->device, this->swapchain, &swapchainImageCount, this->swapchainImages);

	this->renderPass = VulkanRenderer::CreateRenderPass(this->device, swapchainFormat);

	VulkanRenderer::CreateSwapchainImageViews(this->device, swapchainFormat, QUEUE_SLOT_COUNT, this->swapchainImages, this->swapChainImageViews);
	VulkanRenderer::CreateFramebuffers(this->device, this->renderPass, this->window->getExtent().width, this->window->getExtent().height, QUEUE_SLOT_COUNT, 
		this->swapChainImageViews, this->framebuffer);

	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = this->queueFamilyIndex;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	vkCreateCommandPool(this->device, &commandPoolCreateInfo, nullptr, &this->commandPool);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandBufferCount = QUEUE_SLOT_COUNT + 1;
	commandBufferAllocateInfo.commandPool = this->commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer commandBuffers[QUEUE_SLOT_COUNT + 1];

	vkAllocateCommandBuffers(this->device, &commandBufferAllocateInfo, commandBuffers);

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		this->commandBuffers[i] = commandBuffers[i];
	}

	this->setupCommandBuffer = commandBuffers[QUEUE_SLOT_COUNT];

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // We need this so we can wait for them on the first try

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		vkCreateFence(this->device, &fenceCreateInfo, nullptr, &this->frameFences[i]);
		vkCreateSemaphore(this->device, &semaphoreCreateInfo, nullptr, &this->imageAcquiredSemaphores[i]);
		vkCreateSemaphore(this->device, &semaphoreCreateInfo, nullptr, &this->renderingCompleteSemaphores[i]);
	}
}

///////////////////////////////////////////////////////////////////////////////
VulkanRenderer::~VulkanRenderer()
{
	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		vkDestroyFence(this->device, this->frameFences[i], nullptr);
    vkDestroySemaphore(this->device, this->imageAcquiredSemaphores[i], nullptr);
	  vkDestroySemaphore(this->device, this->renderingCompleteSemaphores[i], nullptr);
	}

	vkDestroyRenderPass(this->device, this->renderPass, nullptr);

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		vkDestroyFramebuffer(this->device, this->framebuffer[i], nullptr);
		vkDestroyImageView(this->device, this->swapChainImageViews[i], nullptr);
	}

	vkDestroyCommandPool(this->device, this->commandPool, nullptr);

	vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);
	vkDestroySurfaceKHR(this->instance, this->surface, nullptr);

#ifdef _DEBUG
	CleanupDebugCallback(instance_, debugCallback_, importTable_.get());
#endif

	vkDestroyDevice(this->device, nullptr);
	vkDestroyInstance(this->instance, nullptr);
}

///////////////////////////////////////////////////////////////////////////////
void VulkanRenderer::Render(const int frameCount)
{
	if (this->IsInitialized() == false)
	{		
		return; // just bail out if the user does not have a compatible Vulkan driver
	}

	for (int i = 0; i < frameCount; ++i)
	{
		vkAcquireNextImageKHR(this->device, this->swapchain, UINT64_MAX, this->imageAcquiredSemaphores[this->currentBackBuffer], VK_NULL_HANDLE, &this->currentBackBuffer);

		vkWaitForFences(this->device, 1, &this->frameFences[this->currentBackBuffer], VK_TRUE, UINT64_MAX);
		vkResetFences(this->device, 1, &this->frameFences[this->currentBackBuffer]);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		
		vkBeginCommandBuffer(this->commandBuffers[this->currentBackBuffer], &beginInfo);

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.framebuffer = this->framebuffer[this->currentBackBuffer];
		renderPassBeginInfo.renderArea.extent.width = this->window->getExtent().width;
		renderPassBeginInfo.renderArea.extent.height = this->window->getExtent().height;
		renderPassBeginInfo.renderPass = this->renderPass;

		VkClearValue clearValue = {};

		clearValue.color.float32[0] = 0.042f;
		clearValue.color.float32[1] = 0.042f;
		clearValue.color.float32[2] = 0.042f;
		clearValue.color.float32[3] = 1.0f;

		renderPassBeginInfo.pClearValues = &clearValue;
		renderPassBeginInfo.clearValueCount = 1;

		vkCmdBeginRenderPass(this->commandBuffers[this->currentBackBuffer], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		this->rendererSystem->RenderImpl(this->commandBuffers[this->currentBackBuffer]);

		vkCmdEndRenderPass(this->commandBuffers[this->currentBackBuffer]);
		vkEndCommandBuffer(this->commandBuffers[this->currentBackBuffer]);

		// Submit rendering work to the graphics queue
		const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &this->imageAcquiredSemaphores[this->currentBackBuffer];
		submitInfo.pWaitDstStageMask = &waitDstStageMask;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &this->commandBuffers[this->currentBackBuffer];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &this->renderingCompleteSemaphores[this->currentBackBuffer];
		vkQueueSubmit(this->queue, 1, &submitInfo, VK_NULL_HANDLE);

		// Submit present operation to present queue
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &this->renderingCompleteSemaphores[this->currentBackBuffer];
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &this->swapchain;
		presentInfo.pImageIndices = &this->currentBackBuffer;
		vkQueuePresentKHR(this->queue, &presentInfo);

		vkQueueSubmit(this->queue, 0, nullptr, this->frameFences[this->currentBackBuffer]);
	};

	// Wait for all rendering to finish
	vkWaitForFences(this->device, 3, this->frameFences, VK_TRUE, UINT64_MAX);
}

///////////////////////////////////////////////////////////////////////////////
void VulkanRenderer::PrepareRender()
{
	vkResetFences(this->device, 1, &this->frameFences[0]);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer(this->setupCommandBuffer, &beginInfo);

	this->rendererSystem->InitializeImpl(this->setupCommandBuffer, this->renderPass);

	vkEndCommandBuffer(this->setupCommandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &this->setupCommandBuffer;
	vkQueueSubmit(this->queue, 1, &submitInfo, this->frameFences[0]);

	vkWaitForFences(this->device, 1, &this->frameFences[0], VK_TRUE, UINT64_MAX);
}

///////////////////////////////////////////////////////////////////////////////
void VulkanRenderer::CleanUpRender()
{
	this->rendererSystem->ShutdownImpl();
}