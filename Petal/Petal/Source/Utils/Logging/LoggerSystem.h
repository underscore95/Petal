#pragma once

#include "Logger.h"

namespace Petal {
    class Engine;

    class LoggerSystem {
    public:
        explicit LoggerSystem(Engine &engine);

    public:
        void RegisterLogger(std::shared_ptr<Logger> logger);

        std::shared_ptr<Logger> GetLogger(const std::string &name);

        std::shared_ptr<Logger> CreateLogger(const std::string &name, LogLevel logLevel = LogLevel::Verbose);

    public:
        static constexpr const char* OTHER_LOGGER = "Other";
        static constexpr const char* WINDOW_LOGGER = "Window";
        static constexpr const char* GRAPHICS_LOGGER = "Graphics";
        static constexpr const char* GPU_MEMORY_LOGGER = "GPUMemory";
        static constexpr const char* GRAPHICS_API_LOGGER = "GraphicsAPI";
        static constexpr const char* SCHEDULER_LOGGER = "Scheduler";
        static constexpr std::array ENGINE_LOGGERS = {
            OTHER_LOGGER,
            WINDOW_LOGGER,
            GRAPHICS_LOGGER,
            GRAPHICS_API_LOGGER,
            SCHEDULER_LOGGER,
            GPU_MEMORY_LOGGER
        };

    private:
        Engine &m_engine;
        std::unordered_map<std::string, std::shared_ptr<Logger> > m_loggers;
    };
} // Petal
