#pragma once

#include "../window/window.hpp"
#include "../device/device.hpp"
#include "../game_object/game_object.hpp"
#include "../renderer/renderer.hpp"
#include "../descriptor/descriptor.hpp"
#include "../renderer_system/simple_texture_render_system.hpp"
#include "../renderer_system/simple_render_system.hpp"
#include "../renderer_system/point_light_system.hpp"

#include <memory>
#include <vector>

#define APP_TITLE "Testing Vulkan"

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

			EngineWindow window{WIDTH, HEIGHT, APP_TITLE};
			EngineDevice device{window};
			
			std::shared_ptr<EngineRenderer> renderer{};
			std::shared_ptr<EngineSimpleRenderSystem> simpleRenderSystem{};
			std::shared_ptr<EngineSimpleTextureRenderSystem> textureRenderSystem{};
			std::shared_ptr<EnginePointLightRenderSystem> pointLightRenderSystem{};
			// std::shared_ptr<EngineSimpleTextureRenderSystem> renderSystem{};

			std::vector<EngineGameObject> gameObjects;
	};
}