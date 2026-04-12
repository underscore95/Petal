#include "Window.h"
#include <GLFW/glfw3.h>

namespace Petal {
    Window::Window(
        Engine &engine,
        glm::ivec2 size,
        const std::string &title
    ) : m_engine(engine) {
        m_title = title;
        m_handle = glfwCreateWindow(size.x, size.y, m_title.c_str(), nullptr, nullptr);
    }

    Window::~Window() {
        glfwDestroyWindow(static_cast<GLFWwindow *>(m_handle));
    }

    void Window::Update() {
    }

    void Window::Render() {
    }

    const std::string &Window::GetTitle() {
        return m_title;
    }
} // Petal
