#pragma once

#include "../renderer_system/simple_texture_render_system.hpp"
#include "../window/window.hpp"
#include "../device/device.hpp"
#include "../game_object/game_object.hpp"
#include "../renderer/renderer.hpp"
#include "../descriptor/descriptor.hpp"

#include <memory>
#include <vector>

namespace nugiEngine {
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
			void init();

			EngineWindow window{WIDTH, HEIGHT, "Testing vulkan"};
			EngineDevice device{window};
			EngineRenderer renderer{window, device};
			
			std::shared_ptr<EngineRenderer> renderer{};
			std::shared_ptr<EngineSimpleTextureRenderSystem> renderSystem{};

			std::vector<EngineGameObject> gameObjects;
	};
}