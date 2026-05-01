#pragma once

#include "IntermediateShaderResource.h"
#include "ShaderAsset.h"
#include "slang.h"

namespace Petal {
    class RenderingSystem;
    struct ShaderResource;

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
        Optional<IntermediateShaderResource> CompileSlangShader(const ShaderAsset &asset);

    private:
        void TryLogDiagnosticBlob(Slang::ComPtr<slang::IBlob> blob) const;

        Result CreateGlobalSession();

        Result CreateSession();

        Result ReflectResourceTypes(slang::VariableLayoutReflection *variableLayout, std::unordered_map<ShaderType, slang::IMetadata *> fullMetadata, std::vector<ShaderResource> &resources);

    private:
        Engine &m_engine;
        RenderingSystem &m_renderingSystem;
        std::shared_ptr<Logger> m_logger;
        Slang::ComPtr<slang::IGlobalSession> m_globalSession;
        Slang::ComPtr<slang::ISession> m_session;
    };
} // Petal
