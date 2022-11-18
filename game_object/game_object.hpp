#pragma once

#include "../model/model.hpp"

#include <memory>

struct Transform2DObject
{
	glm::vec2 translation{};
	glm::vec2 scale{1.0f, 1.0f};

	glm::mat2 mat2() { 
		glm::mat2 scaleMat{{scale.x, 0.0f}, {0.0f, scale.y}};
		return scaleMat;
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

		Transform2DObject transform2d;
		std::shared_ptr<EngineModel> model{};
		glm::vec3 color{};



	private:

		id_t objectId;
	};
	
}