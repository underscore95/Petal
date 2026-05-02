#pragma once

#include "FormatString.h"

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

        static constexpr std::array<const char *, NumValues> Colors = {
            "\033[37m", // Verbose - white
            "\033[36m", // Info - cyan
            "\033[33m", // Warn - yellow
            "\033[31m" // Error - red
        };
    };
} // Petal

PETAL_MAKE_FORMATTABLE(
    Petal::LogLevel, level,
    std::format("{}", Petal::LogLevelEnum::Names[static_cast<int>(level)])
);
