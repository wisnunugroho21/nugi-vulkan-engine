#pragma once

#include "../window/window.hpp"
#include "../device/device.hpp"
#include "../game_object/game_object.hpp"
#include "../renderer/renderer.hpp"
#include "../descriptor/descriptor.hpp"
#include "../renderer_system/simple_texture_render_system.hpp"
#include "../renderer_system/simple_render_system.hpp"
#include "../renderer_system/point_light_render_system.hpp"
#include "../renderer_sub/sub_renderer.hpp"

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
			void recreateSubRendererAndSubsystem();

			EngineWindow window{WIDTH, HEIGHT, APP_TITLE};
			EngineDevice device{window};
			
			std::unique_ptr<EngineRenderer> renderer{};
			std::unique_ptr<EngineSubRenderer> subRenderer{};

			std::unique_ptr<EngineSimpleRenderSystem> simpleRenderSystem{};
			std::unique_ptr<EngineSimpleTextureRenderSystem> textureRenderSystem{};
			std::unique_ptr<EnginePointLightRenderSystem> pointLightRenderSystem{};

			std::vector<std::shared_ptr<EngineGameObject>> gameObjects;
	};
}