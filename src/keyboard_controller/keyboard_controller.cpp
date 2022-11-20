#include "keyboard_controller.hpp"

namespace nugiEngine {

  void EngineKeyboardController::moveInPlaceXZ(GLFWwindow* window, float dt, EngineGameObject& gameObject) {
    glm::vec3 rotate{0};

    if (glfwGetKey(window, keymaps.lookRight) == GLFW_PRESS) rotate.y += 1.0f;
    if (glfwGetKey(window, keymaps.lookLeft) == GLFW_PRESS) rotate.y -= 1.0f;
    if (glfwGetKey(window, keymaps.lookUp) == GLFW_PRESS) rotate.x += 1.0f;
    if (glfwGetKey(window, keymaps.lookDown) == GLFW_PRESS) rotate.x -= 1.0f;

    if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
      gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
    }

    gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
    gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

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


