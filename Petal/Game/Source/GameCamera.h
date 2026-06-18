#pragma once

#include "Petal.h"

using namespace Petal;

class GameCamera {
public:
    explicit GameCamera(
        std::shared_ptr<Window> window,
        std::shared_ptr<Renderer> renderer,
        std::shared_ptr<Logger> logger
    );

    void Update(float dt);

    Camera &GetCamera();

private:
    Petal::Camera m_camera;
    std::shared_ptr<Window> m_window;
    std::shared_ptr<Renderer> m_renderer;
    std::shared_ptr<Logger> m_logger;
};
