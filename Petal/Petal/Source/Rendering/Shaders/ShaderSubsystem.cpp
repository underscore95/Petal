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

    Result ShaderSubsystem::CompileSlangShader(
        const ShaderAsset &asset,
        IntermediateShaderResource &out
    ) {
        Timer timer;

        bool isOutputEmpty = !out.SlangModules.empty()
                             || !out.LinkedPrograms.empty()
                             || !out.SPIRV.empty();
        PETAL_CHECK_COND(
            isOutputEmpty,
            Result::SLANG_SHADER_COMPILATION_FAILED,
            m_logger,
            "Failed to load shader modules because the output resource was not empty."
        );

        for (const std::pair<const ShaderType, std::filesystem::path> &sourceFile : asset.SourceFiles) {
            // Compile shader
            ShaderType type = sourceFile.first;
            const std::filesystem::path &path = sourceFile.second;

            ComPtr<IBlob> diagnosticsBlob;
            std::string moduleName = path.filename().string();
            std::string modulePath = path.string();
            std::string source = FileUtils::Read(path, m_logger);

            ComPtr<IModule> slangModule = ComPtr<IModule>(
                m_session->loadModuleFromSourceString(
                    moduleName.c_str(),
                    modulePath.c_str(),
                    source.c_str(),
                    diagnosticsBlob.writeRef()
                )
            );

            // Check success
            TryLogDiagnosticBlob(diagnosticsBlob);
            PETAL_CHECK_COND(!slangModule, Result::SLANG_SHADER_COMPILATION_FAILED, m_logger, "Failed to load shader module {}", modulePath);

            out.SlangModules[type] = slangModule;

            // Link
            ComPtr<IComponentType> linkedProgram;
            diagnosticsBlob = {};
            SlangResult result = slangModule->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
            TryLogDiagnosticBlob(diagnosticsBlob);
            PETAL_CHECK_COND(
                SLANG_FAILED(result),
                Result::SLANG_SHADER_COMPILATION_FAILED,
                m_logger,
                "Failed to link code for shader {}: {}", modulePath, result
            );

            out.LinkedPrograms[type] = linkedProgram;

            // Compile to SPIRV
            ComPtr<IBlob> spirvCode;
            diagnosticsBlob = {};
            result = linkedProgram->getEntryPointCode(0, 0, spirvCode.writeRef(), diagnosticsBlob.writeRef());
            TryLogDiagnosticBlob(diagnosticsBlob);
            PETAL_CHECK_COND(
                SLANG_FAILED(result),
                Result::SLANG_SHADER_COMPILATION_FAILED,
                m_logger,
                "Failed to compile linked code for shader {}: {}", modulePath, result
            );
        }

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
