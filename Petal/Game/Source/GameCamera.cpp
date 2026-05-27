#include "GameCamera.h"

GameCamera::GameCamera(std::shared_ptr<Window> window)
    : m_camera(
          Camera::Projection::Perspective(glm::radians(90.0f), window->GetDimensions(), 0.1, 1000),
          {0, 0, -5},
          {0, 0, 1}
      ),
      m_window(window) {
}

void GameCamera::Update(float dt) {
    constexpr float lookSensitivity = 0.0025f;
    constexpr float moveSpeed = 5.0f;

    glm::vec3 movement(0.0f);

    if (m_window->IsKeyHeld(Key::W))
        movement.z += 1.0f;

    if (m_window->IsKeyHeld(Key::S))
        movement.z -= 1.0f;

    if (m_window->IsKeyHeld(Key::D))
        movement.x += 1.0f;

    if (m_window->IsKeyHeld(Key::A))
        movement.x -= 1.0f;

    if (m_window->IsKeyHeld(Key::E))
        movement.y += 1.0f;

    if (m_window->IsKeyHeld(Key::Q))
        movement.y -= 1.0f;

    if (movement != glm::vec3{0, 0, 0}) {
        movement = glm::normalize(movement);
        m_camera.Move(movement * moveSpeed * dt);
    }

    if (m_window->IsMouseButtonHeld(MouseButton::RIGHT)) {
        glm::vec2 mouseDelta = m_window->GetMouseDelta();

        glm::vec3 rotation = m_camera.GetRotation();

        rotation.y -= mouseDelta.x * lookSensitivity;
        rotation.x -= mouseDelta.y * lookSensitivity;

        constexpr float maxPitch = glm::radians(89.0f);

        rotation.x = glm::clamp(
            rotation.x,
            -maxPitch,
            maxPitch
        );

        m_camera.SetRotation(rotation);
    }
}

Petal::Camera &GameCamera::GetCamera() {
    return m_camera;
}
