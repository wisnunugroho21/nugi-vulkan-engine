#pragma once

#include "../game_object/game_object.hpp"
#include "../window/window.hpp"

namespace nugiEngine {
  class EngineMouseController
  {
  public:
    struct KeyMappings {
      int leftButton = GLFW_MOUSE_BUTTON_LEFT;
      int rightButton = GLFW_MOUSE_BUTTON_RIGHT;
    };

    void rotateInPlaceXZ(GLFWwindow* window, float dt, EngineGameObject& gameObject);

    KeyMappings keymaps{};
    float lookSpeed{2.0f};

    double lastDragged_x = 0;
    double lastDragged_y = 0;
  };
  
} // namespace nugiEngine
