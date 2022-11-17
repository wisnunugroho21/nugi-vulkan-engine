#include "VulkanRendererSystem.hpp"

#include <vector>

struct MemoryTypeInfo
{
    bool deviceLocal = false;
    bool hostVisible = false;
    bool hostCoherent = false;
    bool hostCached = false;
    bool lazilyAllocated = false;

    struct Heap
    {
        uint64_t size = 0;
        bool deviceLocal = false;
    };

    Heap heap;
    int index;
};

class VulkanQuadRenderer : public VulkanRendererSystem
{
public:
	VulkanQuadRenderer(const VkDevice &device, const VkPhysicalDevice &physicalDevice) : VulkanRendererSystem(device, physicalDevice) {}
	
	static VkShaderModule LoadShader(VkDevice device, const std::vector<char>& code);
	static VkPipelineLayout CreatePipelineLayout(VkDevice device);
	static VkBuffer AllocateBuffer (VkDevice device, const int size, const VkBufferUsageFlagBits bits);
	static VkDeviceMemory AllocateMemory(const std::vector<MemoryTypeInfo>& memoryInfos, VkDevice device, const int size, bool* isHostCoherent);
	static std::vector<MemoryTypeInfo> EnumerateHeaps (VkPhysicalDevice device);
	static VkPipeline CreatePipeline(VkDevice device, VkRenderPass renderPass, VkPipelineLayout layout, VkShaderModule vertexShader, 
		VkShaderModule fragmentShader, VkExtent2D viewportSize);

private:
	void CreatePipelineStateObject(const char* vertShaderFilename, const char* fragShaderFilename, VkRenderPass renderPass);
	void CreateMeshBuffers(VkCommandBuffer uploadCommandList);
	void RenderImpl(VkCommandBuffer &commandList) override;
	void InitializeImpl(VkCommandBuffer &uploadCommandList, VkRenderPass &renderPass) override;
	void ShutdownImpl() override;

	VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkBuffer indexBuffer = VK_NULL_HANDLE;

	VkShaderModule vertexShader = VK_NULL_HANDLE;
	VkShaderModule fragmentShader = VK_NULL_HANDLE;

	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
};