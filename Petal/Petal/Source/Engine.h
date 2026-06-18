#pragma once
#include "AppInfo/AppInfo.h"
#include "AppInfo/EngineInfo.h"

namespace Petal {
    class LoggerSystem;
    class MemorySystem;
    class WindowSystem;
    class GraphicsSystem;
    class Scheduler;

    class Engine {
    public:
        Engine();

        ~Engine();

    public:
        MemorySystem &GetMemorySystem() const;

        LoggerSystem &GetLoggerSystem() const;

        Scheduler &GetScheduler() const;

        WindowSystem &GetWindowSystem() const;

        GraphicsSystem &GetGraphicsSystem() const;

        const EngineInfo &GetEngineInfo() const;

        const AppInfo &GetAppInfo() const;

        void Update();

        void LateUpdate();

        void Render();

    private:
        EngineInfo m_engineInfo;
        AppInfo m_appInfo;
        std::unique_ptr<MemorySystem> m_memorySystem;
        std::shared_ptr<LoggerSystem> m_loggerSystem;
        std::unique_ptr<Scheduler> m_scheduler;
        std::shared_ptr<WindowSystem> m_windowSystem;
        std::shared_ptr<GraphicsSystem> m_graphicsSystem;
    };
} // Petal
