#pragma once
#include "Ref.h"

namespace Petal {
    class Engine;

    class MemorySystem {
    public:
        explicit MemorySystem(Engine &engine);

    public:
        template<typename T, typename... Args>
        [[nodiscard]] Ref<T> New(Args &&... args) {
            return std::make_shared<T>(args...);
        }

    private:
        Engine &m_engine;
    };
} // Petal
