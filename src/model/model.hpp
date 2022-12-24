#pragma once

#include "../device/device.hpp"
#include "../buffer/buffer.hpp"
#include "../command/command_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>
#include <memory>

namespace nugiEngine
{
	struct Vertex {
		glm::vec3 position{};
		glm::vec3 color{};
		glm::vec3 normal{};
		glm::vec2 uv{};

		static std::vector<VkVertexInputBindingDescription> getVertexBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions();

		bool operator == (const Vertex &other) const {
			return this->position == other.position && this->color == other.color && this->normal == other.normal 
				&& this->uv == other.uv;
		}
	};

	struct ModelData
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};

		glm::vec3 minAABB{0.f};
		glm::vec3 maxAABB{0.f};

		void loadModel(const std::string &filePath);

		private:
			glm::vec3 findMaximumNumber(std::vector<Vertex> vertices);
			glm::vec3 findMinimumNumber(std::vector<Vertex> vertices);
	};

	class EngineModel
	{
	public:
		EngineModel(EngineDevice &device, const ModelData &data);
		~EngineModel();

		EngineModel(const EngineModel&) = delete;
		EngineModel& operator = (const EngineModel&) = delete;

		std::shared_ptr<EngineBuffer> getVertexBuffer() const { return this->vertexBuffer; }
		std::shared_ptr<EngineBuffer> getIndexBuffer() const { return this->indexBuffer; }
		std::shared_ptr<EngineBuffer> getAABBBuffer() const { return this->aabbBuffer; }
		uint32_t getVertextCount() const { return this->vertextCount; }
		uint32_t getIndexCount() const { return this->indexCount; }

		static std::unique_ptr<EngineModel> createModelFromFile(EngineDevice &device, const std::string &filePath);

		void bind(std::shared_ptr<EngineCommandBuffer> commandBuffer);
		void draw(std::shared_ptr<EngineCommandBuffer> commandBuffer);
		
	private:
		EngineDevice &engineDevice;
		
		std::shared_ptr<EngineBuffer> vertexBuffer;
		uint32_t vertextCount;

		std::shared_ptr<EngineBuffer> indexBuffer;
		uint32_t indexCount;

		std::shared_ptr<EngineBuffer> aabbBuffer;

		bool hasIndexBuffer = false;
		uint64_t deviceAddress = 0;

		void createVertexBuffers(const std::vector<Vertex> &vertices);
		void createIndexBuffer(const std::vector<uint32_t> &indices);
		void createAABBBuffer(glm::vec3 maxPos, glm::vec3 minPos);
	};
} // namespace nugiEngine
