#pragma once

#include "Logger.h"
#include "Memory/Ref.h"

namespace Petal {
    class Engine;

    class LoggerSystem {
    public:
        explicit LoggerSystem(Engine &engine);

    public:
        void RegisterLogger(Ref<Logger> logger);

        Ref<Logger> GetLogger(const std::string &name);

    public:
        static constexpr const char* OTHER_LOGGER = "Other";
        static constexpr std::array ENGINE_LOGGERS = {
            OTHER_LOGGER
        };

    private:
        Engine &m_engine;
        std::unordered_map<std::string, Ref<Logger> > m_loggers;
    };
} // Petal
