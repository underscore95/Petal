#include "Camera.h"

namespace Petal {
    Camera::Projection::Projection(const glm::mat4x4 &matrix)
        : Matrix(matrix) {
    }

    Camera::Projection Camera::Projection::Perspective(
        float fov,
        glm::vec2 screenSize,
        float near,
        float far
    ) {
        assert(screenSize.y != 0);
        return Projection{glm::perspective(fov, screenSize.x / screenSize.y, near, far)};
    }

    Camera::Camera(
        Projection projection,
        glm::vec3 position,
        glm::vec3 forward,
        glm::vec3 up
    )
        : m_position(position),
          m_projMatrix(projection.Matrix) {
        m_forward = glm::normalize(forward);
        m_right = glm::normalize(glm::cross(m_forward, up));
        m_up = glm::normalize(glm::cross(m_right, m_forward));

        m_pitchYawRoll.x = std::asin(glm::clamp(m_forward.y, -1.0f, 1.0f));
        m_pitchYawRoll.y = std::atan2(m_forward.x, -m_forward.z);
        m_pitchYawRoll.z = 0.0f;

        UpdateViewMatrix();
    }

    glm::vec3 Camera::GetPosition() const {
        return m_position;
    }

    void Camera::SetPosition(glm::vec3 newPosition) {
        m_position = newPosition;

        UpdateViewMatrix();
    }

    void Camera::Move(glm::vec3 delta) {
        m_position += delta;

        UpdateViewMatrix();
    }

    glm::vec3 Camera::GetRotation() const {
        return m_pitchYawRoll;
    }

    void Camera::SetRotation(glm::vec3 pitchYawRoll) {
        m_pitchYawRoll = pitchYawRoll;

        glm::vec3 forward;
        forward.x = std::cos(m_pitchYawRoll.x) * std::sin(m_pitchYawRoll.y);
        forward.y = std::sin(m_pitchYawRoll.x);
        forward.z = -std::cos(m_pitchYawRoll.x) * std::cos(m_pitchYawRoll.y);

        m_forward = glm::normalize(forward);

        glm::vec3 worldUp(0.0f, 1.0f, 0.0f);

        m_right = glm::normalize(glm::cross(m_forward, worldUp));
        m_up = glm::normalize(glm::cross(m_right, m_forward));

        if (m_pitchYawRoll.z != 0.0f) {
            glm::mat4 rollMatrix =
                    glm::rotate(glm::mat4(1.0f), m_pitchYawRoll.z, m_forward);

            m_right = glm::normalize(
                glm::vec3(rollMatrix * glm::vec4(m_right, 0.0f))
            );

            m_up = glm::normalize(
                glm::vec3(rollMatrix * glm::vec4(m_up, 0.0f))
            );
        }

        UpdateViewMatrix();
    }

    void Camera::SetProject(const Projection &projection) {
        m_projMatrix = projection.Matrix;
    }

    glm::mat4x4 Camera::GetViewMatrix() const {
        return m_viewMatrix;
    }

    glm::mat4x4 Camera::GetProjMatrix() const {
        return m_projMatrix;
    }

    void Camera::UpdateViewMatrix() {
        m_viewMatrix = glm::lookAt(
            m_position,
            m_position + m_forward,
            m_up
        );
    }
} // Petal
