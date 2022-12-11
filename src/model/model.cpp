#include "model.hpp"
#include "../utils/utils.hpp"

#include <cstring>
#include <iostream>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

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

	EngineModel::~EngineModel() {}

	std::unique_ptr<EngineModel> EngineModel::createModelFromFile(EngineDevice &device, const std::string &filePath) {
		ModelData modelData;
		modelData.loadModel(filePath);

		return std::make_unique<EngineModel>(device, modelData);
	}

	void EngineModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
		this->vertextCount = static_cast<uint32_t>(vertices.size());
		assert(vertextCount >= 3 && "Vertex count must be at least 3");

		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertextCount;
		uint32_t vertexSize = sizeof(vertices[0]);

		EngineBuffer stagingBuffer {
			this->engineDevice,
			vertexSize,
			this->vertextCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void *) vertices.data());

		this->vertexBuffer = std::make_unique<EngineBuffer>(
			this->engineDevice,
			vertexSize,
			this->vertextCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		this->vertexBuffer->copyBuffer(stagingBuffer.getBuffer(), bufferSize);
	}

	void EngineModel::createIndexBuffer(const std::vector<uint32_t> &indices) { 
		this->indexCount = static_cast<uint32_t>(indices.size());
		this->hasIndexBuffer = this->indexCount > 0;

		if (!this->hasIndexBuffer) {
			return;
		}

		VkDeviceSize bufferSize = sizeof(indices[0]) * this->indexCount;
		uint32_t indexSize = sizeof(indices[0]);

		EngineBuffer stagingBuffer {
			this->engineDevice,
			indexSize,
			this->indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void *) indices.data());

		this->indexBuffer = std::make_unique<EngineBuffer>(
			this->engineDevice,
			indexSize,
			this->indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		this->indexBuffer->copyBuffer(stagingBuffer.getBuffer(), bufferSize);
	}

	void EngineModel::bind(std::shared_ptr<EngineCommandBuffer> commandBuffer) {
		VkBuffer buffers[] = {this->vertexBuffer->getBuffer()};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer->getCommandBuffer(), 0, 1, buffers, offsets);

		if (this->hasIndexBuffer) {
			vkCmdBindIndexBuffer(commandBuffer->getCommandBuffer(), this->indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
	}

	void EngineModel::draw(std::shared_ptr<EngineCommandBuffer> commandBuffer) {
		if (this->hasIndexBuffer) {
			vkCmdDrawIndexed(commandBuffer->getCommandBuffer(), this->indexCount, 1, 0, 0, 0);
		} else {
			vkCmdDraw(commandBuffer->getCommandBuffer(), this->vertextCount, 1, 0, 0);
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
		std::vector<VkVertexInputAttributeDescription> attributeDescription(4);
		attributeDescription[0].binding = 0;
		attributeDescription[0].location = 0;
		attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[0].offset = offsetof(Vertex, position);

		attributeDescription[1].binding = 0;
		attributeDescription[1].location = 1;
		attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[1].offset = offsetof(Vertex, color);

		attributeDescription[2].binding = 0;
		attributeDescription[2].location = 2;
		attributeDescription[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[2].offset = offsetof(Vertex, normal);

		attributeDescription[3].binding = 0;
		attributeDescription[3].location = 3;
		attributeDescription[3].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescription[3].offset = offsetof(Vertex, uv);
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

					vertex.color = {
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2]
					};
				}

				if (index.normal_index >= 0) {
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}

				if (index.texcoord_index >= 0) {
					vertex.uv = { // temoirary. for OBJ object only
						attrib.texcoords[2 * index.texcoord_index + 0],
    				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
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

