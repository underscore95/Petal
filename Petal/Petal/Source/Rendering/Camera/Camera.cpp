#include "Camera.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

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
        Projection proj = Projection{glm::perspective(fov, screenSize.x / screenSize.y, near, far)};
        proj.Matrix[1][1] *= -1.0f; // flip for vulkan
        return proj;
    }

    Camera::Camera(
        Projection projection,
        glm::vec3 position,
        glm::vec3 up
    )
        : m_position(position),
          m_projMatrix(projection.Matrix),
          m_worldUp(up) {
        SetRotation(glm::vec2{0.0f, glm::pi<float>() / 2.0f});
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

    glm::vec2 Camera::GetRotation() const {
        return m_pitchYaw;
    }

    void Camera::SetRotation(glm::vec2 pitchYaw) {
        m_pitchYaw = pitchYaw;

        float pitch = pitchYaw.x;
        float yaw = pitchYaw.y;

        glm::vec3 forward = {
            glm::cos(pitch) * glm::cos(yaw),
            glm::sin(pitch),
            glm::cos(pitch) * glm::sin(yaw)
        };

        UpdateDirection(forward);
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

    glm::vec3 Camera::GetUp() const {
        return m_up;
    }

    glm::vec3 Camera::GetLeft() const {
        return m_left;
    }

    glm::vec3 Camera::GetForward() const {
        return m_forward;
    }

    void Camera::UpdateViewMatrix() {
        m_viewMatrix = glm::lookAt(
            m_position,
            m_position + m_forward,
            m_up
        );
    }

    void Camera::UpdateDirection(const glm::vec3 &forward) {
        m_forward = glm::normalize(forward);
        m_left = glm::normalize(glm::cross(m_worldUp, m_forward));
        m_up = glm::normalize(glm::cross(m_forward, m_left));

        UpdateViewMatrix();
    }
} // Petal
