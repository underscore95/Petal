#include "Window.h"
#include <GLFW/glfw3.h>

#include "Engine.h"

namespace Petal {
    Window::Window(
        Engine &engine,
        glm::ivec2 size,
        const std::string &title
    ) : m_engine(engine) {
        m_title = title;
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Don't create surface/swap chain
        m_handle = glfwCreateWindow(size.x, size.y, m_title.c_str(), nullptr, nullptr);
        glfwMakeContextCurrent(static_cast<GLFWwindow *>(m_handle));
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

    bool Window::WantsToClose() const {
        return glfwWindowShouldClose(static_cast<GLFWwindow *>(m_handle));
    }

    Result Window::CreateSurface(const void *instance, void *surfaceOut) const {
        VkResult result = glfwCreateWindowSurface(
            *static_cast<const VkInstance *>(instance),
            static_cast<GLFWwindow *>(m_handle), nullptr,
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
} // Petal
