#include "ShaderSubsystem.h"

#include "File/FileUtils.h"
#include "Timing/Timer.h"

using namespace slang;
using namespace Slang;

namespace Petal {
    struct IntermediateShaderResource;

    ShaderSubsystem::ShaderSubsystem(
        Engine &engine,
        RenderingSystem &renderingSystem,
        std::shared_ptr<Logger> logger,
        Result &resultOut
    )
        : m_engine(engine),
          m_renderingSystem(renderingSystem),
          m_logger(logger) {
        resultOut = CreateGlobalSession();
        if (resultOut != Result::SUCCESS) return;

        resultOut = CreateSession();
        if (resultOut != Result::SUCCESS) return;

        resultOut = Result::SUCCESS;
        m_logger->Verbose("ShaderSystem initialized");
    }

    ShaderSubsystem::~ShaderSubsystem() {
    }

    void ShaderSubsystem::TryLogDiagnosticBlob(ComPtr<IBlob> blob) const {
        if (!blob) return;
        m_logger->Error("[Slang Error] {}", static_cast<const char *>(blob->getBufferPointer()));
    }

    Optional<IntermediateShaderResource> ShaderSubsystem::CompileSlangShader(
        const ShaderAsset &asset
    ) {
        Timer timer;

        IntermediateShaderResource out = {};

        // Load module
        ComPtr<IBlob> diagnosticsBlob;
        std::string moduleName = asset.Source.filename().string();
        std::string modulePath = asset.Source.string();
        std::string source = FileUtils::Read(asset.Source, m_logger);

        ComPtr<IModule> slangModule = ComPtr<IModule>(
            m_session->loadModuleFromSourceString(
                moduleName.c_str(),
                modulePath.c_str(),
                source.c_str(),
                diagnosticsBlob.writeRef()
            )
        );

        TryLogDiagnosticBlob(diagnosticsBlob);
        PETAL_CHECK_COND(!slangModule, Result::SLANG_SHADER_COMPILATION_FAILED, m_logger, "Failed to load shader module {}", modulePath);

        // Find entry points
        std::vector<IComponentType *> components;
        components.reserve(1 + asset.Shaders.size());
        components.push_back(slangModule);

        // Keep a reference to the entry points so they can't be destroyed after the loop ends
        std::vector<ComPtr<IEntryPoint> > entryPoints;
        entryPoints.reserve(asset.Shaders.size());

        for (const std::pair<const ShaderType, ShaderInfo> &shader : asset.Shaders) {
            ShaderType type = shader.first;
            const std::string &entryPointFunctionName = shader.second.EntryPoint;

            // Find entry point
            ComPtr<IEntryPoint> entryPoint;
            diagnosticsBlob = {};
            SlangResult result = slangModule->findEntryPointByName(entryPointFunctionName.c_str(), entryPoint.writeRef());
            TryLogDiagnosticBlob(diagnosticsBlob);
            PETAL_CHECK_COND(
                SLANG_FAILED(result) || !entryPoint,
                Result::SLANG_SHADER_COMPILATION_FAILED,
                m_logger,
                "Failed to find entry point {} in {}", entryPointFunctionName, modulePath
            );

            components.push_back(entryPoint);
            entryPoints.push_back(entryPoint);
        }

        // Create composite program
        ComPtr<IComponentType> program;
        diagnosticsBlob = {};
        SlangResult result = m_session->createCompositeComponentType(
            components.data(),
            components.size(),
            program.writeRef(),
            diagnosticsBlob.writeRef()
        );
        TryLogDiagnosticBlob(diagnosticsBlob);
        PETAL_CHECK_COND(
            SLANG_FAILED(result) || !program,
            Result::SLANG_SHADER_COMPILATION_FAILED,
            m_logger,
            "Failed to compose program for shader {}: {}", modulePath, result
        );

        // Compile
        ComPtr<IBlob> spirvCode;
        diagnosticsBlob = {};
        result = program->getEntryPointCode(0, 0, spirvCode.writeRef(), diagnosticsBlob.writeRef());
        TryLogDiagnosticBlob(diagnosticsBlob);
        PETAL_CHECK_COND(
            SLANG_FAILED(result) || !spirvCode,
            Result::SLANG_SHADER_COMPILATION_FAILED,
            m_logger,
            "Failed to compile linked code for shader {}: {}", modulePath, result
        );

        out.SPIRV = spirvCode;

        m_logger->Verbose("Compiled shader in {} ms", timer.MillisSinceStart());
        return Result::SUCCESS;
    }

    Result ShaderSubsystem::CreateGlobalSession() {
        SlangResult result = createGlobalSession(m_globalSession.writeRef());
        PETAL_CHECK_COND(SLANG_FAILED(result) || !m_globalSession, Result::SLANG_INIT_FAILED, m_logger, "Failed to create global session: {}", result);
        return Result::SUCCESS;
    }

    Result ShaderSubsystem::CreateSession() {
        SessionDesc sessionDesc = {};

        TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = m_globalSession->findProfile("spirv_1_5");

        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;

        std::vector<PreprocessorMacroDesc> preprocessorMacroDesc = {
#ifndef NDEBUG
            PreprocessorMacroDesc{"NDEBUG", "1"}
#endif
        };
        sessionDesc.preprocessorMacros = preprocessorMacroDesc.data();
        sessionDesc.preprocessorMacroCount = static_cast<glm::u32>(preprocessorMacroDesc.size());

        std::vector options =
        {
            CompilerOptionEntry{
                CompilerOptionName::EmitSpirvDirectly, {CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
            }
        };
        sessionDesc.compilerOptionEntries = options.data();
        sessionDesc.compilerOptionEntryCount = options.size();

        SlangResult result = m_globalSession->createSession(sessionDesc, m_session.writeRef());
        PETAL_CHECK_COND(SLANG_FAILED(result) || !m_session, Result::SLANG_INIT_FAILED, m_logger, "Failed to create session: {}", result);
        return Result::SUCCESS;
    }
} // Petal
