#pragma once

#include "../device/device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace nugiEngine
{
	struct Vertex {
		glm::vec2 position;
		glm::vec3 color;

		static std::vector<VkVertexInputBindingDescription> getVertexBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions();
	};

	class EngineModel
	{
	private:
		EngineDevice &engineDevice;
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertextCount;

		void createVertexBuffers(const std::vector<Vertex> vertices);

	public:
		EngineModel(EngineDevice &device, const std::vector<Vertex> vertices);
		~EngineModel();

		EngineModel(const EngineModel&) = delete;
		EngineModel& operator = (const EngineModel&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
	};
} // namespace nugiEngine
