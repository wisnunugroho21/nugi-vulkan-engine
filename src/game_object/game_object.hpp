#pragma once

#include "../model/model.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <memory>

struct TransformComponent {
	glm::vec3 translation{};
	glm::vec3 scale{1.0f, 1.0f, 1.0f};
	glm::vec3 rotation{};

	glm::mat4 mat4() {
		auto transform = glm::translate(glm::mat4{1.0f}, translation);

		transform = glm::rotate(transform, rotation.y, {0.0f, 1.0f, 0.0f});
		transform = glm::rotate(transform, rotation.x, {1.0f, 0.0f, 0.0f});
		transform = glm::rotate(transform, rotation.z, {0.0f, 0.0f, 1.0f});

		transform = glm::scale(transform, scale);
		return transform;
	}
};

namespace nugiEngine {
	class EngineGameObject
	{
	public:
		using id_t = unsigned int;

		static EngineGameObject createGameObject() {
			static id_t currentId = 0;
			return EngineGameObject{currentId++};
		}

		EngineGameObject(const EngineGameObject &) = delete;
		EngineGameObject& operator = (const EngineGameObject &) = delete;
		EngineGameObject(EngineGameObject &&) = default;
		EngineGameObject& operator = (EngineGameObject &&) = default;

		EngineGameObject(id_t id) : objectId{id} {}

		id_t getId() { return this->objectId; }

		TransformComponent transform{};
		std::shared_ptr<EngineModel> model{};
		glm::vec3 color{};

	private:
		id_t objectId;
	};
	
}