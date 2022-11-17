#include <vulkan/vulkan.h>
#include <memory>

#include "../window/glfw_window.hpp"
#include "VulkanRendererSystem.hpp"

struct SwapchainFormatColorSpace
{
    VkFormat format;
    VkColorSpaceKHR colorSpace;
};

struct ImportTable;

class VulkanRenderer
{
public:
	VulkanRenderer(std::shared_ptr<GlfwAppWindow> window);
	~VulkanRenderer();

	VulkanRenderer(const VulkanRenderer&) = delete;
	VulkanRenderer& operator= (const VulkanRenderer&) = delete;

	int getQueueSlot() const { return this->currentBackBuffer; }
	VkDevice getLogicalDevice() const { this->device; }
	VkPhysicalDevice getPhysicalDevice() const { this->physicalDevice; }

	void setRendererSystem(std::shared_ptr<VulkanRendererSystem> rendererSystem) { this->rendererSystem = rendererSystem; }
	void waitIdle() { vkDeviceWaitIdle(this->device); }
	bool IsInitialized() { return (this->instance != VK_NULL_HANDLE && this->device != VK_NULL_HANDLE); }

	virtual void PrepareRender();
	virtual void Render(const int frameCount);
	virtual void CleanUpRender();

	static std::vector<const char*> GetDebugInstanceLayerNames();
	static std::vector<const char*> GetDebugInstanceExtensionNames();
	static std::vector<const char*> GetDebugDeviceLayerNames(VkPhysicalDevice device);
	static void FindPhysicalDeviceWithGraphicsQueue(const std::vector<VkPhysicalDevice>& physicalDevices, VkPhysicalDevice* outputDevice, int* outputGraphicsQueueIndex);
	static SwapchainFormatColorSpace GetSwapchainFormatAndColorspace(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, ImportTable* importTable);
	
	static VkInstance CreateInstance();
	static void CreateDeviceAndQueue(VkInstance instance, VkDevice* outputDevice, VkQueue* outputQueue, int* outputQueueIndex, VkPhysicalDevice* outputPhysicalDevice);
	static VkRenderPass CreateRenderPass(VkDevice device, VkFormat swapchainFormat);
	static void CreateFramebuffers(VkDevice device, VkRenderPass renderPass, const int width, const int height, const int count, const VkImageView* imageViews, VkFramebuffer* framebuffers);
	static void CreateSwapchainImageViews(VkDevice device, VkFormat format, const int count, const VkImage* images, VkImageView* imageViews);
	static VkSwapchainKHR CreateSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, const int surfaceWidth, const int surfaceHeight, const int backbufferCount, ImportTable* importTable, VkFormat* swapchainFormat);
	static VkSurfaceKHR CreateSurface(VkInstance instance, GlfwAppWindow *window);

	#ifdef _DEBUG
	VkDebugReportCallbackEXT SetupDebugCallback(VkInstance instance, VulkanSample::ImportTable* importTable);
	void CleanupDebugCallback(VkInstance instance, VkDebugReportCallbackEXT callback, VulkanSample::ImportTable* importTable);
	#endif
	

protected:
	static const int QUEUE_SLOT_COUNT = 3;
	static int GetQueueSlotCount() { return QUEUE_SLOT_COUNT; }

	int queueFamilyIndex = -1;
	VkViewport viewport;

	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkInstance instance = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkQueue queue = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkRenderPass renderPass = VK_NULL_HANDLE;

	std::unique_ptr<ImportTable> importTable;

	VkFence frameFences[QUEUE_SLOT_COUNT];
	VkImage swapchainImages[QUEUE_SLOT_COUNT];
	VkImageView swapChainImageViews[QUEUE_SLOT_COUNT];
	VkFramebuffer framebuffer[QUEUE_SLOT_COUNT];

	std::shared_ptr<GlfwAppWindow> window;
	std::shared_ptr<VulkanRendererSystem> rendererSystem;

	// sync object
	std::vector<VkSemaphore> imageAcquiredSemaphores;
	std::vector<VkSemaphore> renderingCompleteSemaphores;

private:
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffers[QUEUE_SLOT_COUNT];
    VkCommandBuffer setupCommandBuffer;
    uint32_t currentBackBuffer = 0;

#ifdef _DEBUG
    VkDebugReportCallbackEXT debugCallback;
#endif
};
