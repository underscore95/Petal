#include "Window.h"
#include <GLFW/glfw3.h>

#include "Engine.h"

namespace Petal {
    static GLFWwindow *GLFWHandle(void *handle) {
        return static_cast<GLFWwindow *>(handle);
    }

    void Window::GLFW_KeyCallback(GLFWwindow *pWindow, int key, int scanCode, int action, int mods) {
        auto window = static_cast<Window *>(glfwGetWindowUserPointer(pWindow));
        assert(window);

        window->KeyCallback(key, scanCode, action, mods);
    }

    void Window::GLFW_MouseCallback(GLFWwindow *pWindow, double xPos, double yPos) {
        auto window = static_cast<Window *>(glfwGetWindowUserPointer(pWindow));
        assert(window);

        window->MouseCallback(xPos, yPos);
    }

    void Window::GLFW_MouseButtonCallback(GLFWwindow *pWindow, int button, int action, int mods) {
        auto window = static_cast<Window *>(glfwGetWindowUserPointer(pWindow));
        assert(window);

        window->MouseButtonCallback(button, action, mods);
    }

    void Window::GLFW_ScrollCallback(GLFWwindow *pWindow, double xOffset, double yOffset) {
        auto window = static_cast<Window *>(glfwGetWindowUserPointer(pWindow));
        assert(window);

        window->ScrollCallback(xOffset, yOffset);
    }

    Window::Window(
        Engine &engine,
        glm::ivec2 size,
        const std::string &title
    ) : m_engine(engine) {
        m_title = title;
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Don't create surface/swap chain
        glfwWindowHint(GLFW_RESIZABLE, true);

        m_handle = glfwCreateWindow(size.x, size.y, m_title.c_str(), nullptr, nullptr);
        glfwMakeContextCurrent(GLFWHandle(m_handle));

        glfwSetWindowUserPointer(GLFWHandle(m_handle), this);

        glfwSetKeyCallback(GLFWHandle(m_handle), GLFW_KeyCallback);
        glfwSetCursorPosCallback(GLFWHandle(m_handle), GLFW_MouseCallback);
        glfwSetMouseButtonCallback(GLFWHandle(m_handle), GLFW_MouseButtonCallback);
        glfwSetScrollCallback(GLFWHandle(m_handle), GLFW_ScrollCallback);
    }

    Window::~Window() {
        glfwDestroyWindow(GLFWHandle(m_handle));
    }

    void Window::Update() {
        // poll events done in window system

        // mouse pos & delta
        m_mouseDelta = m_mouseRawPos - m_mousePos;
        m_mousePos = m_mouseRawPos;

        // mouse scroll
        m_scrollDelta = m_scrollDeltaTemp;
        m_scrollDeltaTemp = {0, 0};
    }

    void Window::LateUpdate() {
        // pressed -> held
        for (Key key : m_pressedKeys) {
            m_heldKeys.insert(key);
        }
        m_pressedKeys.clear();

        for (MouseButton button : m_pressedMouseButtons) {
            m_heldMouseButtons.insert(button);
        }
        m_pressedMouseButtons.clear();
    }

    void Window::KeyCallback(int key, [[maybe_unused]] int scanCode, int action, [[maybe_unused]] int mods) {
        if (action == GLFW_PRESS) {
            m_heldKeys.erase(static_cast<Key>(key));
            m_pressedKeys.insert(static_cast<Key>(key));
        } else if (action == GLFW_RELEASE) {
            m_heldKeys.erase(static_cast<Key>(key));
            m_pressedKeys.erase(static_cast<Key>(key));
        }
    }

    void Window::MouseCallback(double xPos, double yPos) {
        m_mouseRawPos = {xPos, yPos};
    }

    void Window::MouseButtonCallback(int button, int action, [[maybe_unused]] int mods) {
        if (action == GLFW_PRESS) {
            m_heldMouseButtons.erase(static_cast<MouseButton>(button));
            m_pressedMouseButtons.insert(static_cast<MouseButton>(button));
        } else if (action == GLFW_RELEASE) {
            m_heldMouseButtons.erase(static_cast<MouseButton>(button));
            m_pressedMouseButtons.erase(static_cast<MouseButton>(button));
        }
    }

    void Window::ScrollCallback(double xOffset, double yOffset) {
        m_scrollDeltaTemp += glm::vec2{xOffset, yOffset};
    }

    void Window::Render() {
    }

    const std::string &Window::GetTitle() {
        return m_title;
    }

    bool Window::WantsToClose() const {
        return glfwWindowShouldClose(GLFWHandle(m_handle));
    }

    Result Window::CreateSurface(const void *instance, void *surfaceOut) const {
        VkResult result = glfwCreateWindowSurface(
            *static_cast<const VkInstance *>(instance),
            GLFWHandle(m_handle), nullptr,
            static_cast<VkSurfaceKHR *>(surfaceOut)
        );

        PETAL_CHECK_COND(
            result != VK_SUCCESS,
            Result::VULKAN_SURFACE_CREATION_FAILED,
            m_engine.GetLoggerSystem().GetLogger(LoggerSystem::WINDOW_LOGGER),
            "{}",
            result
        );

        return Result::SUCCESS;
    }

    glm::uvec2 Window::GetDimensions() const {
        glm::ivec2 size;
        glfwGetWindowSize(GLFWHandle(m_handle), &size.x, &size.y);
        return size;
    }

    bool Window::IsKeyHeld(Key key) const {
        return IsKeyPressed(key) || m_heldKeys.contains(key);
    }

    bool Window::IsKeyPressed(Key key) const {
        return m_pressedKeys.contains(key);
    }

    bool Window::IsMouseButtonHeld(MouseButton button) const {
        return IsMouseButtonPressed(button) || m_heldMouseButtons.contains(button);
    }

    bool Window::IsMouseButtonPressed(MouseButton button) const {
        return m_pressedMouseButtons.contains(button);
    }

    glm::vec2 Window::GetMousePos() const {
        return m_mousePos;
    }

    glm::vec2 Window::GetMouseDelta() const {
        return m_mouseDelta;
    }

    glm::vec2 Window::GetScrollDelta() const {
        return m_scrollDelta;
    }
} // Petal
