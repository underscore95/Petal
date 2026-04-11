#include "Logger.h"

namespace Petal {
    std::mutex Logger::Mutex;

    Logger::Logger(
        std::string name,
        LogLevel logLevel
    ) : m_name(std::move(name)),
        m_level(logLevel) {
    }

    const std::string &Logger::GetName() const {
        return m_name;
    }

    void Logger::Log(const std::string &message, LogLevel level) {
        if (m_level > level) return;

        std::unique_lock lock(Mutex);
        std::cout << std::format("[{}] ({}) {}", level, m_name, message) << std::endl;
    }
} // Petal
