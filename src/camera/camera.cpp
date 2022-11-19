#include "camera.hpp"

#include <cassert>
#include <limits>

namespace nugiEngine {
  void EngineCamera::setOrthographicProjection(
    float left, float right, float top, float bottom, float near, float far) {
      this->projectionMatrix = glm::mat4{1.0f};
      this->projectionMatrix[0][0] = 2.f / (right - left);
      this->projectionMatrix[1][1] = 2.f / (bottom - top);
      this->projectionMatrix[2][2] = 1.f / (far - near);
      this->projectionMatrix[3][0] = -(right + left) / (right - left);
      this->projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
      this->projectionMatrix[3][2] = -near / (far - near);
    }
    
  void EngineCamera::setPerspectiveProjection(float fovy, float aspect, float near, float far) {
    assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
    const float tanHalfFovy = tan(fovy / 2.f);
    this->projectionMatrix = glm::mat4{0.0f};
    this->projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
    this->projectionMatrix[1][1] = 1.f / (tanHalfFovy);
    this->projectionMatrix[2][2] = far / (far - near);
    this->projectionMatrix[2][3] = 1.f;
    this->projectionMatrix[3][2] = -(far * near) / (far - near);
  }
  
} // namespace nugiEngine

