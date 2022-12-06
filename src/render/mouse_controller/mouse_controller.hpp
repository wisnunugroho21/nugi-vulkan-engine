#pragma once

#include "../game_object/game_object.hpp"
#include "../window/window.hpp"
#include "../camera/camera.hpp"
#include "../../pyshic/ray_caster.hpp"

namespace nugiEngine {
  class EngineMouseController
  {
  public:
    struct KeyMappings {
      int leftButton = GLFW_MOUSE_BUTTON_LEFT;
      int rightButton = GLFW_MOUSE_BUTTON_RIGHT;
    };

    void rotateInPlaceXZ(GLFWwindow* window, float dt, EngineGameObject& gameObject);
    int detectObject(GLFWwindow* window, std::vector<std::shared_ptr<EngineGameObject>> gameObjects, EngineCamera camera, int width, int height);
    static float getNormCoordPosition(float diff, float pos);

    KeyMappings keymaps{};
    float lookSpeed{0.5f};

    bool isDragged = false;
    double firstDragged_x = 0;
    double firstDragged_y = 0;
  };
  
} // namespace nugiEngine
