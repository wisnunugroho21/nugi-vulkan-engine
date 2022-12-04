#include "mouse_controller.hpp"

namespace nugiEngine {

  void EngineMouseController::rotateInPlaceXZ(GLFWwindow* window, float dt, EngineGameObject& gameObject) {
    if (glfwGetMouseButton(window, this->keymaps.rightButton) == GLFW_PRESS) {
      if (!this->isDragged) {
        glfwGetCursorPos(window, &this->firstDragged_x, &this->firstDragged_y);
      }

      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      this->isDragged = true;
    } else if (glfwGetMouseButton(window, this->keymaps.rightButton) == GLFW_RELEASE) {
      this->firstDragged_x = 0;
      this->firstDragged_y = 0;

      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      this->isDragged = false;
    }

    if (this->isDragged) {
      double curDragged_x = 0;
      double curDragged_y = 0;

      glfwGetCursorPos(window, &curDragged_x, &curDragged_y);
      glm::vec3 rotate{ (curDragged_y - this->firstDragged_y), ((curDragged_x - this->firstDragged_x) * -1), 0 };

      if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
        gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
      }

      gameObject.transform.rotation = glm::mod(gameObject.transform.rotation, glm::two_pi<float>());
    }
  }
} // namespace nugiEngine


