#pragma once

#include "../device/device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace nugiEngine
{
	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;

		static std::vector<VkVertexInputBindingDescription> getVertexBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions();
	};

	struct ModelData
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
	};

	class EngineModel
	{
	public:
		EngineModel(EngineDevice &device, const ModelData &data);
		~EngineModel();

		EngineModel(const EngineModel&) = delete;
		EngineModel& operator = (const EngineModel&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
		
	private:
		EngineDevice &engineDevice;
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertextCount;

		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
		uint32_t indexCount;

		bool hasIndexBuffer = false;

		void createVertexBuffers(const std::vector<Vertex> &vertices);
		void createIndexBuffer(const std::vector<uint32_t> &indices);
	};
} // namespace nugiEngine
