#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace nugiEngine {
  class EngineCamera {
    public:
      void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
      void setPerspectiveProjection(float fovy, float aspect_ratio, float near, float far);

      const glm::mat4 getProjectionMatrix() const { return this->projectionMatrix; }

    private:
      glm::mat4 projectionMatrix{1.0f};
  };
} // namespace nugiEngine

