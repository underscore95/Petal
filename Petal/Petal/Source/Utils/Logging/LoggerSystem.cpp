#include "LoggerSystem.h"

#include "Engine.h"
#include "Memory/MemorySystem.h"

namespace Petal {
    LoggerSystem::LoggerSystem(
        Engine &engine
    ) : m_engine(engine) {
        for (auto &loggerName : ENGINE_LOGGERS) {
            RegisterLogger(std::make_shared<Logger>(loggerName, LogLevel::Verbose));
        }
    }

    void LoggerSystem::RegisterLogger(std::shared_ptr<Logger> logger) {
        if (m_loggers.contains(logger->GetName())) {
            logger->Error("Failed to register logger because another logger with the same name already exists.");
            return;
        }

        m_loggers[logger->GetName()] = logger;
    }

    std::shared_ptr<Logger> LoggerSystem::GetLogger(const std::string &name) {
        const auto it = m_loggers.find(name);
        if (it == m_loggers.end()) {
            assert(false);
            return nullptr;
        }

        return it->second;
    }
} // Petal
