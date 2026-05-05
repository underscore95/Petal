#pragma once
#include "AppInfo/AppInfo.h"
#include "AppInfo/EngineInfo.h"

namespace Petal {
    class LoggerSystem;
    class MemorySystem;
    class WindowSystem;
    class GraphicsSystem;

    class Engine {
    public:
        Engine();

        ~Engine();

    public:
        MemorySystem &GetMemorySystem() const;

        LoggerSystem &GetLoggerSystem() const;

        WindowSystem &GetWindowSystem() const;

        GraphicsSystem &GetRenderingSystem() const;

        const EngineInfo &GetEngineInfo() const;

        const AppInfo &GetAppInfo() const;

        void Update();

        void Render();

    private:
        EngineInfo m_engineInfo;
        AppInfo m_appInfo;
        std::unique_ptr<MemorySystem> m_memorySystem;
        std::shared_ptr<LoggerSystem> m_loggerSystem;
        std::shared_ptr<WindowSystem> m_windowSystem;
        std::shared_ptr<GraphicsSystem> m_renderingSystem;
    };
} // Petal
