#pragma once

#include "../model/model.hpp"
#include "../texture/texture.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <memory>

namespace nugiEngine {
	struct TransformComponent {
		glm::vec3 translation{};
		glm::vec3 scale{1.0f, 1.0f, 1.0f};
		glm::vec3 rotation{};

		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};
	
	struct LightComponent {
		float intensity = 1.0f;
		float cutoff = glm::radians(45.0f);
		int type = 0;
		glm::vec3 direction{0.0f};
	};
	

	class EngineGameObject
	{
	public:
		using id_t = unsigned int;

		static EngineGameObject createGameObject() {
			static id_t currentId = 0;
			return EngineGameObject{currentId++};
		}

		static std::shared_ptr<EngineGameObject> createSharedGameObject() {
			static id_t currentId = 0;
			return std::make_shared<EngineGameObject>(currentId++);
		}

		static EngineGameObject createLight(float intensity = 10.0f, float radius = 1.0f, glm::vec3 color = glm::vec3{1.0f}, int lightType = 0, float cutoff = glm::radians(45.0f), glm::vec3 direction = glm::vec3{0.0f});
		static std::shared_ptr<EngineGameObject> createSharedLight(float intensity = 10.0f, float radius = 1.0f, glm::vec3 color = glm::vec3{1.0f}, int lightType = 0, float cutoff = glm::radians(45.0f), glm::vec3 direction = glm::vec3{0.0f});

		EngineGameObject(const EngineGameObject &) = delete;
		EngineGameObject& operator = (const EngineGameObject &) = delete;
		EngineGameObject(EngineGameObject &&) = default;
		EngineGameObject& operator = (EngineGameObject &&) = default;

		EngineGameObject(id_t id) : objectId{id} {}

		id_t getId() { return this->objectId; }

		TransformComponent transform{};
		glm::vec3 color{};

		std::shared_ptr<EngineModel> model{};
		std::shared_ptr<EngineTexture> texture{};
		std::shared_ptr<VkDescriptorSet> textureDescSet{};
		std::unique_ptr<LightComponent> light = nullptr;

	private:
		id_t objectId;
	};
	
}