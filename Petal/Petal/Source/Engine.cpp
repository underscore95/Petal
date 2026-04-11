#include "Engine.h"
#include "pch.h"
#include "Memory/MemorySystem.h"
#include "Logging/LoggerSystem.h"

namespace Petal {
    Engine::Engine() {
        m_memorySystem = std::make_unique<MemorySystem>(*this);
        m_loggerSystem = m_memorySystem->New<LoggerSystem>(*this);
    }

    Engine::~Engine() {
    }

    MemorySystem &Engine::GetMemorySystem() const {
        return *m_memorySystem;
    }

    LoggerSystem &Engine::GetLoggerSystem() const {
        return *m_loggerSystem;
    }
} // Petal
