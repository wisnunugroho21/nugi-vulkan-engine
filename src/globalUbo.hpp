#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace nugiEngine {
  #define MAX_LIGHTS 10

  struct pointLight {
    glm::vec4 position{};
    glm::vec4 color{};
    alignas(16) glm::vec3 direction{};
    alignas(4) float cutoff = 0;
    alignas(4) int type = 0;
  };

  struct GlobalUBO {
    glm::mat4 projection{1.0f};
    glm::mat4 view{1.0f};
    glm::mat4 inverseView{1.0f};
  };

  struct GlobalLight {
    glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f};
    pointLight pointLights[MAX_LIGHTS];
    int numLights;
  };
}