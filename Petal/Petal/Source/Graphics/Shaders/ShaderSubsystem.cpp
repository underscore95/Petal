#include "ShaderSubsystem.h"

#include "File/FileUtils.h"
#include "Timing/Timer.h"

using namespace slang;
using namespace Slang;

namespace Petal {
    ShaderSubsystem::ShaderSubsystem(
        Engine &engine,
        GraphicsSystem &renderingSystem,
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

        std::unordered_map<ShaderType, IMetadata *> metadatas; // Used for reflection later

        // Keep a reference to the entry points so they can't be destroyed after the loop ends
        // and attach some information to them
        struct EntryPointInfo {
            ComPtr<IEntryPoint> Ptr;
            ShaderType Type;
        };
        std::vector<EntryPointInfo> entryPoints;
        entryPoints.reserve(asset.Shaders.size());

        glm::u32 entryPointIndex=0;
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
            entryPoints.emplace_back(entryPoint, type);
            out.ShaderTypes[type] = IntermediateShaderResource::ShaderStage{
                .EntryFunctionName = entryPoint->getFunctionReflection()->getName(),
                .EntryPointIndex = entryPointIndex,
                .SPIRV = nullptr
            };
            m_logger->Verbose("Found shader entry point {} with name {} and index {}", type, out.ShaderTypes[type].EntryFunctionName, out.ShaderTypes[type].EntryPointIndex);

            assert(entryPointIndex + 2 == components.size());
            entryPointIndex++;
        }

        // Create composite program
        ComPtr<IComponentType> program;
        diagnosticsBlob = {};
        SlangResult result = m_session->createCompositeComponentType(
            components.data(),
            static_cast<glm::u32>(components.size()),
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

        // Metadata
        for (size_t i = 0; i < entryPoints.size(); i++) {
            IMetadata *metadata = nullptr;
            diagnosticsBlob = {};
            SlangResult result = program->getEntryPointMetadata(
                static_cast<SlangInt>(i),
                0,
                &metadata,
                diagnosticsBlob.writeRef()
            );
            TryLogDiagnosticBlob(diagnosticsBlob);

            PETAL_CHECK_COND(
                SLANG_FAILED(result) || !metadata,
                Result::SLANG_SHADER_COMPILATION_FAILED,
                m_logger,
                "Failed to get metadata for entry point {} ({}) in {}", i, entryPoints[i].Type, modulePath
            );

            metadatas[entryPoints[i].Type] = metadata;
        }

        // Compile
        for (std::pair<const ShaderType, IntermediateShaderResource::ShaderStage> &pair : out.ShaderTypes) {
            ComPtr<IBlob> spirvCode;
            diagnosticsBlob = {};
            result = program->getEntryPointCode(pair.second.EntryPointIndex, 0, spirvCode.writeRef(), diagnosticsBlob.writeRef());
            TryLogDiagnosticBlob(diagnosticsBlob);
            PETAL_CHECK_COND(
                SLANG_FAILED(result) || !spirvCode,
                Result::SLANG_SHADER_COMPILATION_FAILED,
                m_logger,
                "Failed to compile linked code for shader {}: {}", modulePath, result
            );

            pair.second.SPIRV = spirvCode;
        }

        // Reflection
        diagnosticsBlob = {};
        ProgramLayout *programLayout = program->getLayout(0, diagnosticsBlob.writeRef());
        TryLogDiagnosticBlob(diagnosticsBlob);

        Result petalResult = ReflectResourceTypes(programLayout->getGlobalParamsVarLayout(), metadatas, out.Resources);
        PETAL_CHECK_COND(
            petalResult != Result::SUCCESS,
            petalResult,
            m_logger,
            "Failed to reflect resource types for shader: {}", modulePath
        );

        m_logger->Verbose("Compiled shader in {} ms", timer.MillisSinceStart());
        return out;
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
            },
            // Don't rename entry points to "main" in the SPIRV
            {
                slang::CompilerOptionName::VulkanUseEntryPointName, slang::CompilerOptionValue{CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
            }
        };
        sessionDesc.compilerOptionEntries = options.data();
        sessionDesc.compilerOptionEntryCount = options.size();

        SlangResult result = m_globalSession->createSession(sessionDesc, m_session.writeRef());
        PETAL_CHECK_COND(SLANG_FAILED(result) || !m_session, Result::SLANG_INIT_FAILED, m_logger, "Failed to create session: {}", result);
        return Result::SUCCESS;
    }

    Result ShaderSubsystem::ReflectResourceTypes(
        slang::VariableLayoutReflection *variableLayout,
        std::unordered_map<ShaderType, slang::IMetadata *> fullMetadata,
        std::vector<ShaderResource> &resources
    ) {
        Optional<ResourceType> resourceType(Result::PETAL_SHADER_REFLECTION_INVALID_TYPE);
        TypeReflection::Kind variableType = variableLayout->getTypeLayout()->getKind();
        SlangResourceShape shape = variableLayout->getType()->getResourceShape();

        // Recursively iterate fields in structs
        if (variableType == TypeReflection::Kind::Struct) {
            if (variableLayout->getTypeLayout()->getFieldCount() == 0) {
                m_logger->Warn("Struct {} with no fields in shader", variableLayout->getName());
            }

            for (glm::u32 i = 0; i < variableLayout->getTypeLayout()->getFieldCount(); i++) {
                VariableLayoutReflection *field = variableLayout->getTypeLayout()->getFieldByIndex(i);
                Result result = ReflectResourceTypes(field, fullMetadata, resources);
                PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);
            }
            return Result::SUCCESS;
        } else if (variableType == TypeReflection::Kind::Resource) {
            // Don't know the type...
            if (shape == SlangResourceShape::SLANG_STRUCTURED_BUFFER) {
                resourceType = ResourceType::STORAGE_BUFFER;
            }
        } else if (variableType == TypeReflection::Kind::ConstantBuffer) {
            // todo support constant buffer / ubo
        }

        if (resourceType.IsEmpty()) {
            m_logger->Error("Unsupported field type {} (field name: {}, shape: {}, kind: {}) in shader.", variableType, shape, variableType, variableLayout->getName());
            return Result::PETAL_SHADER_REFLECTION_FAILED;
        }

        // Get resource information
        ShaderResource resource = {
            .Name = variableLayout->getName(),
            .Type = *resourceType.Value(),
            .BindingIndex = variableLayout->getBindingIndex(),
            .BindingSet = variableLayout->getBindingSpace(),
            .Stages = {}
        };

        // Check what stages use the parameter
        for (const std::pair<const ShaderType, IMetadata *> metadataPair : fullMetadata) {
            ShaderType shaderType = metadataPair.first;
            IMetadata *metadata = metadataPair.second;

            bool used;
            SlangResult result = metadata->isParameterLocationUsed(
                static_cast<SlangParameterCategory>(variableLayout->getCategory()), // I don't know why there is two enums in slang for this
                resource.BindingSet,
                resource.BindingIndex,
                used
            );

            PETAL_CHECK_COND(
                SLANG_FAILED(result),
                Result::PETAL_SHADER_REFLECTION_FAILED,
                m_logger,
                "Failed to check if parameter {} is used in shader: {}", resource.Name, result
            );

            if (used) {
                resource.Stages.push_back(shaderType);
            }
        }

        resources.push_back(resource);

        return Result::SUCCESS;
    }
} // Petal
