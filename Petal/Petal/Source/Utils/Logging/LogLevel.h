#pragma once

#include "pch.h"

namespace Petal {
    enum class LogLevel {
        Verbose, Info, Warn, Error
    };

    class LogLevelEnum {
    public:
        LogLevelEnum() = delete;

    public:
        static constexpr int NumValues = 4;
        static constexpr std::array<const char *, NumValues> Names = {
            "Verbose", "Info", "Warn", "Error"
        };
    };
} // Petal

// Source - https://stackoverflow.com/a/59914918
// Posted by vitaut, modified by community. See post 'Timeline' for change history
// Retrieved 2026-04-11, License - CC BY-SA 4.0

template<>
struct std::formatter<Petal::LogLevel> : std::formatter<std::string> {
    auto format(Petal::LogLevel instance, format_context &ctx) const {
        return formatter<string>::format(
            std::format("{}", Petal::LogLevelEnum::Names[static_cast<int>(instance)]), ctx);
    }
};
