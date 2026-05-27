#pragma once

#include "Petal.h"

using namespace Petal;

class GameCamera {
public:
    explicit GameCamera(std::shared_ptr<Window> window);

    void Update(float dt);

    Camera &GetCamera();

private:
    Petal::Camera m_camera;
    std::shared_ptr<Window> m_window;
};
