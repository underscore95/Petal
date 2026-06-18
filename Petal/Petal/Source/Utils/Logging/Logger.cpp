#include "Logger.h"

namespace Petal {
    std::mutex Logger::Mutex;

    Logger::Logger(
        std::string name,
        LogLevel logLevel,
        bool mute
    ) : m_name(std::move(name)),
        m_level(logLevel),
        m_muted(mute) {
    }

    const std::string &Logger::GetName() const {
        return m_name;
    }

    void Logger::SetLevel(LogLevel level) {
        m_level = level;
    }

    void Logger::SetMuted(bool muted) {
        m_muted = muted;
    }

    void Logger::Log(const std::string &message, LogLevel level) {
        static constexpr const char *ResetColour = "\033[0m";
        if (m_level > level) return;
        if (m_muted) return;

        std::unique_lock lock(Mutex);
        std::cout << std::format(
            "{}[{}] ({}) {}{}",
            LogLevelEnum::Colors[static_cast<size_t>(level)],
            level,
            m_name,
            message,
            ResetColour
        ) << std::endl;
    }
} // Petal
