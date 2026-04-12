#pragma once
#include "Memory/Ref.h"
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

        void Render();

        [[nodiscard]] Ref<Window> OpenWindow(
            glm::ivec2 size = {1280, 720},
            const std::string &title = "Petal Engine"
        );

        void CloseWindow(Ref<Window> window);

    private:
        Engine &m_engine;
        Ref<Logger> m_logger;
        std::unordered_set<Ref<Window> > m_windows;
    };
} // Petal
