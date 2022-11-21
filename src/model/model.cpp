#include "model.hpp"
#include "../utils/utils.hpp"

#include <cstring>
#include <iostream>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj/tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std {
	template<>
	struct hash<nugiEngine::Vertex> {
		size_t operator () (nugiEngine::Vertex const &vertex) const {
			size_t seed = 0;
			nugiEngine::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
} // namespace std

namespace nugiEngine {
	EngineModel::EngineModel(EngineDevice &device, const ModelData &datas) : engineDevice{device} {
		this->createVertexBuffers(datas.vertices);
		this->createIndexBuffer(datas.indices);
	}

	EngineModel::~EngineModel() {
		vkDestroyBuffer(this->engineDevice.getLogicalDevice(), this->vertexBuffer, nullptr);
		vkFreeMemory(this->engineDevice.getLogicalDevice(), this->vertexBufferMemory, nullptr);

		if (this->hasIndexBuffer) {
			vkDestroyBuffer(this->engineDevice.getLogicalDevice(), this->indexBuffer, nullptr);
			vkFreeMemory(this->engineDevice.getLogicalDevice(), this->indexBufferMemory, nullptr);
		}
	}

	std::unique_ptr<EngineModel> EngineModel::createModelFromFile(EngineDevice &device, const std::string &filePath) {
		ModelData modelData;
		modelData.loadModel(filePath);

		std::cout << "Vertex Count: " << modelData.vertices.size() << '\n';

		return std::make_unique<EngineModel>(device, modelData);
	}

	void EngineModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
		this->vertextCount = static_cast<uint32_t>(vertices.size());
		assert(vertextCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertextCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		this->engineDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		void* data;
		vkMapMemory(this->engineDevice.getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(this->engineDevice.getLogicalDevice(), stagingBufferMemory);

		this->engineDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			this->vertexBuffer,
			this->vertexBufferMemory
		);
		
		this->engineDevice.copyBuffer(stagingBuffer, this->vertexBuffer, bufferSize);

		vkDestroyBuffer(this->engineDevice.getLogicalDevice(), stagingBuffer, nullptr);
		vkFreeMemory(this->engineDevice.getLogicalDevice(), stagingBufferMemory, nullptr);
	}

	void EngineModel::createIndexBuffer(const std::vector<uint32_t> &indices) { 
		this->indexCount = static_cast<uint32_t>(indices.size());
		this->hasIndexBuffer = this->indexCount > 0;

		if (!this->hasIndexBuffer) {
			return;
		}

		VkDeviceSize bufferSize = sizeof(indices[0]) * this->indexCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		this->engineDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		void* data;
		vkMapMemory(this->engineDevice.getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(this->engineDevice.getLogicalDevice(), stagingBufferMemory);

		this->engineDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			this->indexBuffer,
			this->indexBufferMemory
		);
		
		this->engineDevice.copyBuffer(stagingBuffer, this->indexBuffer, bufferSize);

		vkDestroyBuffer(this->engineDevice.getLogicalDevice(), stagingBuffer, nullptr);
		vkFreeMemory(this->engineDevice.getLogicalDevice(), stagingBufferMemory, nullptr);
	}

	void EngineModel::bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = {this->vertexBuffer};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (this->hasIndexBuffer) {
			vkCmdBindIndexBuffer(commandBuffer, this->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		}
	}

	void EngineModel::draw(VkCommandBuffer commandBuffer) {
		if (this->hasIndexBuffer) {
			vkCmdDrawIndexed(commandBuffer, this->indexCount, 1, 0, 0, 0);
		} else {
			vkCmdDraw(commandBuffer, this->vertextCount, 1, 0, 0);
		}
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
		attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[0].offset = offsetof(Vertex, position);

		attributeDescription[1].binding = 0;
		attributeDescription[1].location = 1;
		attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[1].offset = offsetof(Vertex, color);
		return attributeDescription;
	}

	void ModelData::loadModel(const std::string &filePath) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str())) {
			throw std::runtime_error(warn + err);
		}

		this->vertices.clear();
		this->indices.clear();

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};
		for (const auto &shape: shapes) {
			for (const auto &index: shape.mesh.indices) {
				Vertex vertex{};

				if (index.vertex_index >= 0) {
					vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					auto colorIndex = 3 * index.vertex_index + 2;
					if (colorIndex < attrib.colors.size()) {
						vertex.color = {
							attrib.colors[colorIndex - 2],
							attrib.colors[colorIndex - 1],
							attrib.colors[colorIndex - 0]
						};
					} else {
						vertex.color = {1.0f, 1.0f, 1.0f};
					}
				}

				if (index.normal_index >= 0) {
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}

				if (index.texcoord_index >= 0) {
					vertex.uv = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(this->vertices.size());
					this->vertices.push_back(vertex);
				}

				this->indices.push_back(uniqueVertices[vertex]);
			}
		}
	}
    
} // namespace nugiEngine

