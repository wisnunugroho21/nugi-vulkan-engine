#include "mouse_controller.hpp"
#include "../../pyshic/ray_caster.hpp"

#include <iostream>

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

  float EngineMouseController::getNormCoordPosition(float diff, float pos) {
    return pos / diff * 2 - 1;
  }

  int EngineMouseController::detectObject(GLFWwindow* window, std::vector<std::shared_ptr<EngineGameObject>> gameObjects, EngineCamera camera, int width, int height) {
    if (glfwGetMouseButton(window, this->keymaps.rightButton) == GLFW_PRESS) {
      double mouseX = 0;
      double mouseY = 0;

      glfwGetCursorPos(window, &mouseX, &mouseY);

      float XPos = EngineMouseController::getNormCoordPosition(static_cast<float>(width), static_cast<float>(mouseX));
      float YPos = EngineMouseController::getNormCoordPosition(static_cast<float>(height), static_cast<float>(mouseY));

      glm::vec4 mouseOrigin = glm::vec4{XPos, YPos, 0.0f, 1.0f};
      mouseOrigin = camera.getInverseViewMatrix() * camera.getInverseProjectionMatrix() * mouseOrigin;
      glm::vec3 rayOrigin = glm::vec3{mouseOrigin.x, mouseOrigin.y, mouseOrigin.z};

      glm::vec4 mouseDirection = glm::vec4{XPos, YPos, 1.0f, 1.0f};
      mouseDirection = camera.getInverseViewMatrix() * camera.getInverseProjectionMatrix() * mouseDirection;
      glm::vec3 rayDirection = glm::vec3{mouseDirection.x, mouseDirection.y, mouseDirection.z};

      uint32_t objId = rayCasting(rayOrigin, rayDirection, gameObjects);
      std::cerr << objId;
    }
  }
} // namespace nugiEngine


