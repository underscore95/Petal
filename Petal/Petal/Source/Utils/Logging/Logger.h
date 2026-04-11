#pragma once

#include "LogLevel.h"
#include "pch.h"

namespace Petal {
    class Logger {
    public:
        Logger(std::string name, LogLevel logLevel);

    public:
        template<typename... Args>
        void Verbose(const std::string &fmt, Args &&... args) {
            Log(std::vformat(fmt, std::make_format_args(args...)), LogLevel::Verbose);
        }

        template<typename... Args>
        void Info(const std::string &fmt, Args &&... args) {
            Log(std::vformat(fmt, std::make_format_args(args...)), LogLevel::Info);
        }

        template<typename... Args>
        void Warn(const std::string &fmt, Args &&... args) {
            Log(std::vformat(fmt, std::make_format_args(args...)), LogLevel::Warn);
        }

        template<typename... Args>
        void Error(const std::string &fmt, Args &&... args) {
            Log(std::vformat(fmt, std::make_format_args(args...)), LogLevel::Error);
        }

        const std::string& GetName() const;

    private:
        void Log(const std::string &message, LogLevel level);

    private:
        static std::mutex Mutex;

    private:
        std::string m_name;
        LogLevel m_level;
    };
} // Petal
