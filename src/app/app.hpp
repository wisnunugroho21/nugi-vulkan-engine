#pragma once

#include "../window/window.hpp"
#include "../device/device.hpp"
#include "../game_object/game_object.hpp"
#include "../renderer/renderer.hpp"
#include "../descriptor/descriptor.hpp"
#include "../acceleration_structure/bottom_level_acceleration_structure.hpp"
#include "../acceleration_structure/top_level_acceleration_structure.hpp"
#include "../renderer_system/ray_tracing_render_system.hpp"
#include "../device/device_procedures.hpp"

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
			EngineDeviceProcedures deviceProcedure{device};
			
			std::unique_ptr<EngineRenderer> renderer{};
			std::unique_ptr<EngineRayTracingRenderSystem> rayTracingRenderSystem{};

			std::vector<std::shared_ptr<EngineGameObject>> gameObjects;
	};
}