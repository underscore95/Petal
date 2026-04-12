#include "Engine.h"
#include "pch.h"
#include "Petal.h"

namespace Petal {
    Engine::Engine() {
        m_memorySystem = std::make_unique<MemorySystem>(*this);
        m_loggerSystem = m_memorySystem->New<LoggerSystem>(*this);
        m_windowSystem = m_memorySystem->New<WindowSystem>(*this);
    }

    Engine::~Engine() {
    }

    MemorySystem &Engine::GetMemorySystem() const {
        return *m_memorySystem;
    }

    LoggerSystem &Engine::GetLoggerSystem() const {
        return *m_loggerSystem;
    }

    WindowSystem &Engine::GetWindowSystem() const {
        return *m_windowSystem;
    }
} // Petal
