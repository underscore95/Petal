#pragma once

#include "pch.h"

namespace Petal {
    class Engine;

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

        void Render();

        const std::string &GetTitle();

    private:
        Engine &m_engine;
        void *m_handle;
        std::string m_title;
    };
} // Petal
