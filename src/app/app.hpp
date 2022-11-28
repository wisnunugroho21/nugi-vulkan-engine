#pragma once

#include "../window/window.hpp"
#include "../device/device.hpp"
#include "../game_object/game_object.hpp"
#include "../renderer/renderer.hpp"
#include "../descriptor/descriptor.hpp"

#include <memory>
#include <vector>

namespace nugiEngine {
	struct GlobalUBO {
		glm::mat4 projectionView{1.0f};
		glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f};
		glm::vec3 lightPosition{-1.0f};
		alignas(16) glm::vec4 lightColor{-1.0f};
	};

	class EngineApp
	{
		public:
			static constexpr int WIDTH = 800;
			static constexpr int HEIGHT = 600;

			EngineApp();
			~EngineApp();

			EngineApp(const EngineApp&) = delete;
			EngineApp& operator = (const EngineApp&) = delete;

			void run();

		private:
			void loadObjects();

			EngineWindow window{WIDTH, HEIGHT, "Testing vulkan"};
			EngineDevice device{window};
			EngineRenderer renderer{window, device, sizeof(GlobalUBO)};

			std::vector<EngineGameObject> gameObjects;
	};
}