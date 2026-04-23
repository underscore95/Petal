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

    public:
        static constexpr const char* OTHER_LOGGER = "Other";
        static constexpr const char* WINDOW_LOGGER = "Window";
        static constexpr const char* RENDERING_LOGGER = "Rendering";
        static constexpr const char* GRAPHICS_API_LOGGER = "GraphicsAPI";
        static constexpr std::array ENGINE_LOGGERS = {
            OTHER_LOGGER,
            WINDOW_LOGGER,
            RENDERING_LOGGER,
            GRAPHICS_API_LOGGER
        };

    private:
        Engine &m_engine;
        std::unordered_map<std::string, std::shared_ptr<Logger> > m_loggers;
    };
} // Petal
