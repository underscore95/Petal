#pragma once

#include "Common.h"

namespace Petal {
    class Camera {
    public:
        struct Projection {
            explicit Projection(const glm::mat4x4 &matrix);

            glm::mat4x4 Matrix;

            static Projection Perspective(float fov, glm::vec2 screenSize, float near, float far);
        };

    public:
        Camera(
            Projection projection,
            glm::vec3 position,
            glm::vec3 forward,
            glm::vec3 up = {0, 1, 0}
        );

    public:
        glm::vec3 GetPosition() const;

        void SetPosition(glm::vec3 newPosition);

        void Move(glm::vec3 delta);

        glm::vec3 GetRotation() const;

        void SetRotation(glm::vec3 pitchYawRoll);

        void SetProject(const Projection &projection);

        glm::mat4x4 GetViewMatrix() const;

        glm::mat4x4 GetProjMatrix() const;

    private:
        void UpdateViewMatrix();

    private:
        glm::vec3 m_up;
        glm::vec3 m_forward;
        glm::vec3 m_right;
        glm::vec3 m_position;
        glm::vec3 m_pitchYawRoll;
        glm::mat4x4 m_projMatrix;
        glm::mat4x4 m_viewMatrix;
    };
} // Petal
