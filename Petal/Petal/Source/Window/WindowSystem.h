#pragma once

#include "pch.h"

namespace Petal {
    class Engine;
    class Window;
    class Logger;

    class WindowSystem {
    public:
        explicit WindowSystem(
            Engine &engine
        );

        ~WindowSystem();

    public:
        void Update();

        void LateUpdate();

        void Render();

        [[nodiscard]] std::shared_ptr<Window> OpenWindow(
            glm::ivec2 size = {1280, 720},
            const std::string &title = "Petal Engine"
        );

        void CloseWindow(std::shared_ptr<Window> window);

    private:
        Engine &m_engine;
        std::shared_ptr<Logger> m_logger;
        std::unordered_set<std::shared_ptr<Window> > m_windows;
    };
} // Petal
