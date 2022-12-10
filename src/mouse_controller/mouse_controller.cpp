#include "mouse_controller.hpp"

namespace nugiEngine {

  void EngineMouseController::rotateInPlaceXZ(GLFWwindow* window, float dt, EngineGameObject& gameObject) {
    if (glfwGetMouseButton(window, this->keymaps.rightButton) == GLFW_PRESS) {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

      double curDragged_x = 0;
      double curDragged_y = 0;

      glfwGetCursorPos(window, &curDragged_x, &curDragged_y);
      glm::vec3 rotate{ (curDragged_y - this->lastDragged_y), ((curDragged_x - this->lastDragged_x) * -1), 0 };

      this->lastDragged_x = curDragged_x;
      this->lastDragged_y = curDragged_y;

      if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
        gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
      }

      gameObject.transform.rotation = glm::mod(gameObject.transform.rotation, glm::two_pi<float>());
    } else if (glfwGetMouseButton(window, this->keymaps.rightButton) == GLFW_RELEASE) {
      this->lastDragged_x = 0;
      this->lastDragged_y = 0;

      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
  }
} // namespace nugiEngine


