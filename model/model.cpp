#include "model.hpp"

#include <cstring>

namespace nugiEngine
{
	EngineModel::EngineModel(EngineDevice &device, const std::vector<Vertex> vertices) : engineDevice{device} {
		this->createVertexBuffers(vertices);
	}

	EngineModel::~EngineModel() {
		vkDestroyBuffer(this->engineDevice.device(), this->vertexBuffer, nullptr);
		vkFreeMemory(this->engineDevice.device(), this->vertexBufferMemory, nullptr);
	}

	void EngineModel::createVertexBuffers(const std::vector<Vertex> vertices) {
		this->vertextCount = static_cast<uint32_t>(vertices.size());
		assert(vertextCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertextCount;

		this->engineDevice.createBuffer(
				bufferSize,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				this->vertexBuffer,
				this->vertexBufferMemory
		);

		void* data;
		vkMapMemory(this->engineDevice.device(), this->vertexBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(this->engineDevice.device(), this->vertexBufferMemory);
	}

	void EngineModel::bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = {this->vertexBuffer};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

void EngineModel::draw(VkCommandBuffer commandBuffer) {
		vkCmdDraw(commandBuffer, this->vertextCount, 1, 0, 0);
	}

	std::vector<VkVertexInputBindingDescription> Vertex::getVertexBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> Vertex::getVertexAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescription(2);
		attributeDescription[0].binding = 0;
		attributeDescription[0].location = 0;
		attributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescription[0].offset = offsetof(Vertex, position);

		attributeDescription[1].binding = 0;
		attributeDescription[1].location = 1;
		attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[1].offset = offsetof(Vertex, color);
		return attributeDescription;
	}
    
} // namespace nugiEngine

