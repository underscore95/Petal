#pragma once

#include "IntermediateShaderResource.h"
#include "ShaderAsset.h"
#include "slang.h"

namespace Petal {
    class GraphicsSystem;
    struct ShaderResource;

    class ShaderSubsystem {
    public:
        ShaderSubsystem(
            Engine &engine,
            GraphicsSystem &renderingSystem,
            std::shared_ptr<Logger> logger,
            Result &resultOut
        );

        ~ShaderSubsystem();

    public:
        // Compile a Slang shader
        // This converts to SPIRV and performs reflection, but it isn't actually compiled into a Vulkan shader until you compile the intermediate shader using the GraphicsContext.
        //
        Optional<IntermediateShaderResource> CompileSlangShader(const ShaderAsset &asset);

    private:
        void TryLogDiagnosticBlob(Slang::ComPtr<slang::IBlob> blob) const;

        Result CreateGlobalSession();

        Result CreateSession();

        Result ReflectResourceTypes(slang::VariableLayoutReflection *variableLayout, std::unordered_map<ShaderType, slang::IMetadata *> fullMetadata,
                                    std::vector<ShaderResource> &resources);

        Result ReflectVertexInput(IntermediateShaderResource &shader, slang::ProgramLayout *programLayout, glm::u32 vertexEntryPointIndex);

        // If variableLayout is a struct, recursively iterate and push the VariableLayoutReflection of all fields to the vecotr
        // otherwise push variableLayout to the vector
        // this does not clear the output vector!
        void ReflectGetAllFieldsRecursive(
            slang::VariableLayoutReflection *variableLayout,
            std::vector<slang::VariableLayoutReflection *> &fieldsOut
        );

    private:
        Engine &m_engine;
        GraphicsSystem &m_renderingSystem;
        std::shared_ptr<Logger> m_logger;
        Slang::ComPtr<slang::IGlobalSession> m_globalSession;
        Slang::ComPtr<slang::ISession> m_session;
    };
} // Petal
