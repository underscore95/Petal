#pragma once

#include "Common.h"

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

        bool WantsToClose() const;

        Result CreateSurface(const void *instance, void *surfaceOut) const;

    private:
        Engine &m_engine;
        void *m_handle;
        std::string m_title;
    };
} // Petal
