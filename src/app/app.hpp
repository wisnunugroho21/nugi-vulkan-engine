#pragma once

#include "../window/window.hpp"
#include "../device/device.hpp"
#include "../game_object/game_object.hpp"
#include "../descriptor/descriptor.hpp"
#include "../renderer/ray_tracing_renderer.hpp"
#include "../renderer_system/trace_ray_render_system.hpp"
#include "../renderer_system/sampling_ray_render_system.hpp"

#include <memory>
#include <vector>

#define APP_TITLE "Testing Vulkan"

namespace nugiEngine {
	class EngineApp
	{
		public:
			static constexpr int WIDTH = 800;
			static constexpr int HEIGHT = 800;

			EngineApp();
			~EngineApp();

			EngineApp(const EngineApp&) = delete;
			EngineApp& operator = (const EngineApp&) = delete;

			void run();

		private:
			void loadObjects();
			RayTraceUbo updateCamera();
			void recreateSubRendererAndSubsystem();

			EngineWindow window{WIDTH, HEIGHT, APP_TITLE};
			EngineDevice device{window};
			
			std::unique_ptr<EngineRayTraceRenderer> renderer{};
			std::unique_ptr<EngineTraceRayRenderSystem> traceRayRender{};
			std::unique_ptr<EngineSamplingRayRenderSystem> samplingRayRender{};
	};
}