#include "Engine.h"
#include "pch.h"
#include "Petal.h"

namespace Petal {
    Engine::Engine() {
        m_engineInfo = {
            .Name = "Petal",
            .EngineVersion = Version{
                .Variant = PETAL_VERSION_VARIANT,
                .Major = PETAL_VERSION_MAJOR,
                .Minor = PETAL_VERSION_MINOR,
                .Patch = PETAL_VERSION_PATCH,
            }
        };

        m_appInfo = {
            .Name = "TODO",
            .AppVersion = {0, 1, 0, 0}
        };

        m_memorySystem = std::make_unique<MemorySystem>(*this);
        m_loggerSystem = std::make_shared<LoggerSystem>(*this);
        m_scheduler = std::make_unique<Scheduler>(m_loggerSystem->GetLogger(LoggerSystem::SCHEDULER_LOGGER));
        m_windowSystem = std::make_shared<WindowSystem>(*this);
        m_graphicsSystem = std::make_shared<GraphicsSystem>(*this, DeviceRequirements::DEFAULT_API_VERSION);
    }

    Engine::~Engine() {
    }

    Scheduler &Engine::GetScheduler() const {
        return *m_scheduler;
    }

    MemorySystem &Engine::GetMemorySystem() const {
        return *m_memorySystem;
    }

    LoggerSystem &Engine::GetLoggerSystem() const {
        return *m_loggerSystem;
    }

    WindowSystem &Engine::GetWindowSystem() const {
        return *m_windowSystem;
    }

    GraphicsSystem &Engine::GetGraphicsSystem() const {
        return *m_graphicsSystem;
    }

    const EngineInfo &Engine::GetEngineInfo() const {
        return m_engineInfo;
    }

    const AppInfo &Engine::GetAppInfo() const {
        return m_appInfo;
    }

    void Engine::Update() {
        m_windowSystem->Update();
    }

    void Engine::LateUpdate() {
        m_windowSystem->LateUpdate();
    }

    void Engine::Render() {
        m_scheduler->Render();
        m_windowSystem->Render();
    }
} // Petal
