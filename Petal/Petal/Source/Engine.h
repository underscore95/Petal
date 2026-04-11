#pragma once
#include "Memory/Ref.h"

namespace Petal {
    class LoggerSystem;
    class MemorySystem;

    class Engine {
    public:
        Engine();

        ~Engine();

    public:
        MemorySystem& GetMemorySystem() const;
        LoggerSystem& GetLoggerSystem() const;

    private:
        std::unique_ptr<MemorySystem> m_memorySystem;
        Ref<LoggerSystem> m_loggerSystem;
    };
} // Petal
