#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace nugiEngine {
  struct GlobalUBO {
    glm::mat4 projection{1.0f};
    glm::mat4 view{1.0f};
    glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f};
    glm::vec3 lightPosition{-1.0f};
    alignas(16) glm::vec4 lightColor{-1.0f};
  };
}