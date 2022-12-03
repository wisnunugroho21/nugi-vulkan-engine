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
	private:
		id_t objectId;
	};
	
}