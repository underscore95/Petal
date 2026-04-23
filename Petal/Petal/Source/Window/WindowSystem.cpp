#include "WindowSystem.h"

#include "Engine.h"
#include "Window.h"
#include "Logging/LoggerSystem.h"
#include "Memory/MemorySystem.h"
#include <GLFW/glfw3.h>

namespace Petal {
    WindowSystem::WindowSystem(
        Engine &engine
    ) : m_engine(engine) {
        m_logger = engine.GetLoggerSystem().GetLogger(LoggerSystem::WINDOW_LOGGER);
        glfwInit();
    }

    WindowSystem::~WindowSystem() {
        m_windows.clear();
        glfwTerminate();
    }

    void WindowSystem::Update() {
        glfwPollEvents();

        for (auto &window : m_windows) {
            window->Update();
        }
    }

    void WindowSystem::Render() {
        for (auto &window : m_windows) {
            window->Render();
        }
    }

    std::shared_ptr<Window> WindowSystem::OpenWindow(
        glm::ivec2 size,
        const std::string &title
    ) {
        std::shared_ptr<Window> window = std::make_shared<Window>(m_engine, size, title);
        m_windows.insert(window);
        return window;
    }

    void WindowSystem::CloseWindow(std::shared_ptr<Window> window) {
        const auto it = m_windows.find(window);
        if (it == m_windows.end()) {
            m_logger->Error("Failed to close window titled {}", window->GetTitle());
            return;
        }

        m_windows.erase(it);
    }
} // Petal
