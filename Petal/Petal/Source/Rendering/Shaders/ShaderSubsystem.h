#pragma once

#include "IntermediateShaderResource.h"
#include "ShaderAsset.h"
#include "slang.h"

namespace Petal {
    class RenderingSystem;

    class ShaderSubsystem {
    public:
        ShaderSubsystem(
            Engine &engine,
            RenderingSystem &renderingSystem,
            std::shared_ptr<Logger> logger,
            Result &resultOut
        );

        ~ShaderSubsystem();

    public:


        Result CompileSlangShader(const ShaderAsset &asset, IntermediateShaderResource &out);
    private:
        void TryLogDiagnosticBlob(Slang::ComPtr<slang::IBlob> blob) const;
        Result CreateGlobalSession();
        Result CreateSession();

    private:
        Engine &m_engine;
        RenderingSystem &m_renderingSystem;
        std::shared_ptr<Logger> m_logger;
        Slang::ComPtr<slang::IGlobalSession> m_globalSession;
        Slang::ComPtr<slang::ISession> m_session;
    };
} // Petal
