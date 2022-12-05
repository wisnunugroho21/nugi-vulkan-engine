#include "keyboard_controller.hpp"

namespace nugiEngine {

  void EngineKeyboardController::moveInPlaceXZ(GLFWwindow* window, float dt, EngineGameObject& gameObject) {
    float yaw = gameObject.transform.rotation.y;
    const glm::vec3 forwardDir{sin(yaw), 0.0f, cos(yaw)};
    const glm::vec3 rightDir{forwardDir.z, 0.0f, -forwardDir.x};
    const glm::vec3 upDir{0.0f, -1.0f, 0.0};

    glm::vec3 moveDir{0.0f};
    if (glfwGetKey(window, keymaps.moveForward) == GLFW_PRESS) moveDir += forwardDir;
    if (glfwGetKey(window, keymaps.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
    if (glfwGetKey(window, keymaps.moveRight) == GLFW_PRESS) moveDir += rightDir;
    if (glfwGetKey(window, keymaps.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
    if (glfwGetKey(window, keymaps.moveUp) == GLFW_PRESS) moveDir += upDir;
    if (glfwGetKey(window, keymaps.moveDown) == GLFW_PRESS) moveDir -= upDir;

    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
      gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
    }
  }
  
} // namespace nugiEngin 


