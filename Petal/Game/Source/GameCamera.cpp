#include "GameCamera.h"

GameCamera::GameCamera(
    std::shared_ptr<Window> window,
    std::shared_ptr<Renderer> renderer,
    std::shared_ptr<Logger> logger
)
    : m_camera(
          Camera::Projection::Perspective(glm::radians(90.0f), window->GetDimensions(), 0.1, 1000),
          {0, 0, -5}
      ),
      m_window(window),
      m_renderer(renderer),
      m_logger(logger) {
    m_renderer->SetCamera(m_camera);
}

void GameCamera::Update(float dt) {
    constexpr float lookSensitivity = 0.0025f;
    constexpr float moveSpeed = 5.0f;

    glm::vec3 movement(0.0f);

    if (m_window->IsKeyHeld(Key::W))
        movement += m_camera.GetForward();

    if (m_window->IsKeyHeld(Key::S))
        movement -= m_camera.GetForward();

    if (m_window->IsKeyHeld(Key::D))
        movement -= m_camera.GetLeft();

    if (m_window->IsKeyHeld(Key::A))
        movement += m_camera.GetLeft();

    if (m_window->IsKeyHeld(Key::E))
        movement += m_camera.GetUp();

    if (m_window->IsKeyHeld(Key::Q))
        movement -= m_camera.GetUp();

    if (movement != glm::vec3{0, 0, 0}) {
        movement = glm::normalize(movement);
        m_camera.Move(movement * moveSpeed * dt);
    }

    if (m_window->IsMouseButtonHeld(MouseButton::RIGHT)) {
        glm::vec2 mouseDelta = m_window->GetMouseDelta();

        glm::vec2 rotation = m_camera.GetRotation();

        rotation.y += mouseDelta.x * lookSensitivity;
        rotation.x -= mouseDelta.y * lookSensitivity;

        constexpr float maxPitch = glm::radians(89.0f);

        rotation.x = glm::clamp(
            rotation.x,
            -maxPitch,
            maxPitch
        );

        m_camera.SetRotation(rotation);
    }

    m_renderer->SetCamera(m_camera);
}

Petal::Camera &GameCamera::GetCamera() {
    return m_camera;
}
