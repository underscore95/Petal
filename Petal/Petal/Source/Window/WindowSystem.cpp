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

    Ref<Window> WindowSystem::OpenWindow(
        glm::ivec2 size,
        const std::string &title
    ) {
        Ref<Window> window = m_engine.GetMemorySystem().New<Window>(m_engine, size, title);
        m_windows.insert(window);
        return window;
    }

    void WindowSystem::CloseWindow(Ref<Window> window) {
        const auto it = m_windows.find(window);
        if (it == m_windows.end()) {
            m_logger->Error("Failed to close window titled {}", window->GetTitle());
            return;
        }

        m_windows.erase(it);
    }

    Ref<Window> WindowSystem::GetFirstWindow() const {
        for (auto window : m_windows) return window;
        assert(false);
        return nullptr;
    }
} // Petal
