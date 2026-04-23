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
        m_loggerSystem = m_memorySystem->New<LoggerSystem>(*this);
        m_windowSystem = m_memorySystem->New<WindowSystem>(*this);
        m_renderingSystem = m_memorySystem->New<RenderingSystem>(*this, DeviceRequirements::DEFAULT_API_VERSION);
    }

    Engine::~Engine() {
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

    RenderingSystem &Engine::GetRenderingSystem() const {
        return *m_renderingSystem;
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

    void Engine::Render() {
        m_windowSystem->Render();
    }
} // Petal
