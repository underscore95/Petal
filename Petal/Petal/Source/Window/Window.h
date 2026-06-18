#pragma once

#include <GLFW/glfw3.h>

#include "Common.h"
#include "Key.h"
#include "MouseButton.h"

namespace Petal {
    class Engine;

    // https://github.com/underscore95/Spire/blob/main/Spire/Spire/Source/Window/Window.h
    class Window {
    public:
        explicit Window(
            Engine &engine,
            glm::ivec2 size,
            const std::string &title
        );

        ~Window();

    public:
        void Update();

        void LateUpdate();

        void Render();

        const std::string &GetTitle();

        bool WantsToClose() const;

        Result CreateSurface(const void *instance, void *surfaceOut) const;

        glm::uvec2 GetDimensions() const;

        bool IsKeyHeld(Key key) const;

        bool IsKeyPressed(Key key) const;

        bool IsMouseButtonHeld(MouseButton button) const;

        bool IsMouseButtonPressed(MouseButton button) const;

        glm::vec2 GetMousePos() const;

        glm::vec2 GetMouseDelta() const;

        glm::vec2 GetScrollDelta() const;

    private:
        void KeyCallback(int key, int scanCode, int action, int mods);

        void MouseCallback(double xPos, double yPos);

        void MouseButtonCallback(int button, int action, int mods);

        void ScrollCallback(double xOffset, double yOffset);

        static void GLFW_KeyCallback(GLFWwindow *pWindow, int key, int scanCode, int action, int mods);

        static void GLFW_MouseCallback(GLFWwindow *pWindow, double xPos, double yPos);

        static void GLFW_MouseButtonCallback(GLFWwindow *pWindow, int button, int action, int mods);

        static void GLFW_ScrollCallback(GLFWwindow *pWindow, double xOffset, double yOffset);

    private:
        Engine &m_engine;
        void *m_handle;
        std::string m_title;
        std::unordered_set<Key> m_pressedKeys;
        std::unordered_set<Key> m_heldKeys;
        std::unordered_set<MouseButton> m_pressedMouseButtons;
        std::unordered_set<MouseButton> m_heldMouseButtons;
        glm::vec2 m_mousePos;
        glm::vec2 m_mouseRawPos;
        glm::vec2 m_mouseDelta;
        glm::vec2 m_scrollDelta;
        glm::vec2 m_scrollDeltaTemp;
    };
} // Petal
