#pragma once
#include "Memory/Ref.h"

namespace Petal {
    class LoggerSystem;
    class MemorySystem;
    class WindowSystem;

    class Engine {
    public:
        Engine();

        ~Engine();

    public:
        MemorySystem &GetMemorySystem() const;

        LoggerSystem &GetLoggerSystem() const;

        WindowSystem &GetWindowSystem() const;

    private:
        std::unique_ptr<MemorySystem> m_memorySystem;
        Ref<LoggerSystem> m_loggerSystem;
        Ref<WindowSystem> m_windowSystem;
    };
} // Petal
