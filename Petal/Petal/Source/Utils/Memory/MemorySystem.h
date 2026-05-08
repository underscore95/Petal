#pragma once

namespace Petal {
    class Engine;

    class MemorySystem {
    public:
        explicit MemorySystem(Engine &engine);

    public:

    private:
        Engine &m_engine;
    };
} // Petal
