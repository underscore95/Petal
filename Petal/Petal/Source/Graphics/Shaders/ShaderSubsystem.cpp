#include "ShaderSubsystem.h"

#include "File/FileUtils.h"
#include "Timing/Timer.h"
#include "Graphics/Internal/Vulkan.h"

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

        m_logger->Verbose("Compiling shader {}", asset.Source.string());

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

        glm::u32 entryPointIndex = 0;
        Optional<glm::u32> vertexEntryPointIndex = Result::PETAL_OPTIONAL_EMPTY;
        Optional<glm::u32> fragmentEntryPointIndex = Result::PETAL_OPTIONAL_EMPTY;
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

            if (type == ShaderType::VERTEX) {
                assert(vertexEntryPointIndex.IsEmpty());
                vertexEntryPointIndex = entryPointIndex;
            } else if (type == ShaderType::FRAGMENT) {
                assert(fragmentEntryPointIndex.IsEmpty());
                fragmentEntryPointIndex = entryPointIndex;
            }

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

        if (vertexEntryPointIndex.HasValue()) {
            petalResult = ReflectVertexInput(out, programLayout, vertexEntryPointIndex.Value());
            PETAL_CHECK_COND(
                petalResult != Result::SUCCESS,
                petalResult,
                m_logger,
                "Failed to reflect vertex input for shader: {}", modulePath
            );
        }

        if (fragmentEntryPointIndex.HasValue()) {
            petalResult = ReflectFragmentOutput(out, programLayout, fragmentEntryPointIndex.Value());
            PETAL_CHECK_COND(
                petalResult != Result::SUCCESS,
                petalResult,
                m_logger,
                "Failed to reflect fragment output for shader: {}", modulePath
            );
        }

        // Output log
        for (const ShaderResource &resource : out.Resources) {
            m_logger->Verbose("Found resource '{}' ({}) in shaders {} bound to set {} index {}", resource.Name, resource.Type, resource.Stages, resource.BindingSet,
                              resource.BindingIndex);
            if (resource.IsArray) {
                if (resource.IsStaticArray()) m_logger->Verbose(" - '{}' is a static array with size {}", resource.Name, resource.StaticArraySize);
                else m_logger->Verbose(" - '{}' is a dynamically sized array.", resource.Name);
            }
        }

        if (out.VertexType.HasValue()) {
            m_logger->Verbose("Vertex Size: {} ({} Attributes)", out.VertexType->Size, out.VertexType->Attributes.size());
            for (const VertexType::Attribute &attr : out.VertexType->Attributes) {
                m_logger->Verbose(" - Found attribute '{}' with size {} and format {}", attr.Name, attr.Size, attr.Format);
            }
        }

        if (out.FragmentShader.HasValue()) {
            m_logger->Verbose("Fragment Shader - {} Outputs", out.FragmentShader->Outputs.size());
            for (const IntermediateShaderResource::FragmentOutput &fragmentOut : out.FragmentShader->Outputs) {
                m_logger->Verbose(" - Output '{}' ({}x {})", fragmentOut.Name, fragmentOut.NumElements, fragmentOut.Type);
            }
        }

        m_logger->Verbose("Compiled and reflected shader {} in {} ms", asset.Source.string(), timer.MillisSinceStart());
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

        // If array, extract the wrapped type
        const bool isArray = variableType == TypeReflection::Kind::Array;
        glm::u32 staticArraySize = 0;
        if (isArray) {
            TypeLayoutReflection *element = variableLayout->getTypeLayout()->getElementTypeLayout();
            variableType = element->getKind();
            shape = element->getType()->getResourceShape();
            staticArraySize = variableLayout->getTypeLayout()->getElementCount();
        }

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
        }

        if (variableType == TypeReflection::Kind::Resource) {
            // Don't know the type...
            if (shape == SlangResourceShape::SLANG_STRUCTURED_BUFFER) {
                resourceType = ResourceType::STORAGE_BUFFER;
            } else if (shape == (SLANG_TEXTURE_2D | SLANG_TEXTURE_COMBINED_FLAG)) {
                resourceType = ResourceType::COMBINED_SAMPLER;
            }
        } else if (variableType == TypeReflection::Kind::ConstantBuffer) {
            resourceType = ResourceType::CONSTANT_BUFFER;
            TypeLayoutReflection *bufferContents = variableLayout->getTypeLayout()->getElementTypeLayout();
            PETAL_CHECK_COND(
                bufferContents->getName() == nullptr,
                Result::PETAL_SHADER_REFLECTION_FAILED,
                m_logger,
                "Automatically-introduced (or anonymous type) ConstantBuffers are not supported by Petal."
                "\nFix this error by explicitly writing ConstantBuffer<T> MyBuffer instead of using uniforms.",
                bufferContents->getName() == nullptr ? "anonymous" : bufferContents->getName(),
                bufferContents->getFieldCount()
            );
        }

        if (resourceType.IsEmpty()) {
            m_logger->Error("Unsupported field type (shape: {}, kind: {}) in shader.", shape, variableType);
            return Result::PETAL_SHADER_REFLECTION_FAILED;
        }

        // Get resource information
        ShaderResource resource = {
            .Name = variableLayout->getName() == nullptr ? "UnnamedResource" : variableLayout->getName(),
            .Type = resourceType.Value(),
            .BindingIndex = variableLayout->getBindingIndex(),
            .BindingSet = variableLayout->getBindingSpace(),
            .Stages = {},
            .IsArray = isArray,
            .StaticArraySize = staticArraySize
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

    Result ShaderSubsystem::ReflectVertexInput(
        IntermediateShaderResource &shader,
        ProgramLayout *programLayout,
        glm::u32 vertexEntryPointIndex
    ) {
        EntryPointReflection *entryPointReflection = programLayout->getEntryPointByIndex(vertexEntryPointIndex);
        PETAL_CHECK_COND(entryPointReflection->getStage() != SLANG_STAGE_VERTEX, Result::PETAL_SHADER_REFLECTION_FAILED, m_logger, "ReflectVertexInput invalid entry point");
        glm::u32 numParams = entryPointReflection->getParameterCount();

        // find all input params that are vertex varying input
        std::vector<VariableLayoutReflection *> inputs;
        for (glm::u32 i = 0; i < numParams; i++) {
            VariableLayoutReflection *paramLayout = entryPointReflection->getParameterByIndex(i);

            std::string name = paramLayout->getName();
            ParameterCategory category = paramLayout->getCategory();
            if (category != ParameterCategory::VaryingInput) continue;

            ReflectGetAllFieldsRecursive(paramLayout, inputs);
        }

        // store types
        VertexType info = {};
        for (VariableLayoutReflection *variableLayout : inputs) {
            constexpr glm::u32 SCALAR_SIZE = 4;
            TypeReflection::Kind kind = variableLayout->getType()->getKind();
            if (kind == TypeReflection::Kind::Vector) {
                std::array<VkFormat, 3> FLOAT_VECTOR_FORMATS = {VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT};
                constexpr std::array<VkFormat, 3> INT_VECTOR_FORMATS = {
                    VK_FORMAT_R32G32_SINT,
                    VK_FORMAT_R32G32B32_SINT,
                    VK_FORMAT_R32G32B32A32_SINT
                };
                constexpr std::array<VkFormat, 3> UINT_VECTOR_FORMATS = {
                    VK_FORMAT_R32G32_UINT,
                    VK_FORMAT_R32G32B32_UINT,
                    VK_FORMAT_R32G32B32A32_UINT
                };
                glm::u32 elementCount = variableLayout->getType()->getElementCount();
                assert(elementCount >= 2 && elementCount <= 4);
                TypeReflection::ScalarType scalarType = variableLayout->getType()->getElementType()->getScalarType();
                VkFormat format;
                if (scalarType == TypeReflection::Float32) format = FLOAT_VECTOR_FORMATS[elementCount - 2];
                else if (scalarType == TypeReflection::UInt32) format = UINT_VECTOR_FORMATS[elementCount - 2];
                else if (scalarType == TypeReflection::Int32) format = INT_VECTOR_FORMATS[elementCount - 2];
                else {
                    PETAL_ERROR(Result::PETAL_SHADER_REFLECTION_INVALID_TYPE, m_logger, "Unsupported vector with scalar type {} as vertex input", scalarType);
                }
                info.Attributes.push_back({variableLayout->getName(), SCALAR_SIZE * elementCount, format});
            } else if (kind == TypeReflection::Kind::Scalar) {
                TypeReflection::ScalarType scalarType = variableLayout->getType()->getScalarType();
                VkFormat format;
                if (scalarType == TypeReflection::Float32) format = VK_FORMAT_R32_SFLOAT;
                else if (scalarType == TypeReflection::UInt32) format = VK_FORMAT_R32_UINT;
                else if (scalarType == TypeReflection::Int32) format = VK_FORMAT_R32_SINT;
                else {
                    PETAL_ERROR(Result::PETAL_SHADER_REFLECTION_INVALID_TYPE, m_logger, "Unsupported scalar type {} as vertex input", scalarType);
                }
                info.Attributes.push_back({variableLayout->getName(), SCALAR_SIZE, format});
            } else {
                PETAL_ERROR(Result::PETAL_SHADER_REFLECTION_INVALID_TYPE, m_logger, "Unsupported type {} as vertex input", variableLayout->getType()->getName());
            }
            info.Size += info.Attributes.back().Size;
        }

        shader.VertexType = info;

        return Result::SUCCESS;
    }

    Result ShaderSubsystem::ReflectFragmentOutput(
        IntermediateShaderResource &shader,
        slang::ProgramLayout *programLayout,
        glm::u32 fragmentEntryPointIndex
    ) {
        shader.FragmentShader = IntermediateShaderResource::FragmentShaderInfo{};
        EntryPointReflection *entryPointReflection = programLayout->getEntryPointByIndex(fragmentEntryPointIndex);
        PETAL_CHECK_COND(entryPointReflection->getStage() != SLANG_STAGE_FRAGMENT, Result::PETAL_SHADER_REFLECTION_FAILED, m_logger, "ReflectFragmentOutput invalid entry point");

        if (VariableLayoutReflection *output = entryPointReflection->getResultVarLayout()) {
            Result result = ReflectFragmentOutputField(shader, output);
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);
        }

        return Result::SUCCESS;
    }

    static Optional<IntermediateShaderResource::ScalarType> GetPetalScalarType(TypeLayoutReflection *type) {
        switch (type->getScalarType()) {
            case TypeReflection::Float32:
                return IntermediateShaderResource::ScalarType::FLOAT;
            case TypeReflection::Int32:
                return IntermediateShaderResource::ScalarType::INT;
            case TypeReflection::UInt32:
                return IntermediateShaderResource::ScalarType::UNSIGNED_INT;
            default:
                return Result::PETAL_OPTIONAL_EMPTY;
        }
    }

    Result ShaderSubsystem::ReflectFragmentOutputField(
        IntermediateShaderResource &shader,
        slang::VariableLayoutReflection *field
    ) {
        TypeLayoutReflection *fieldType = field->getTypeLayout();

        // Recursively handle struct
        if (fieldType->getKind() == TypeReflection::Kind::Struct) {
            for (glm::u32 i = 0; i < fieldType->getFieldCount(); i++) {
                Result result = ReflectFragmentOutputField(shader, fieldType->getFieldByIndex(i));
                PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);
            }
            return Result::SUCCESS;
        }

        // scalar / vector
        glm::u32 numElements = 1;
        Optional<IntermediateShaderResource::ScalarType> scalarType = Result::PETAL_OPTIONAL_EMPTY;
        if (fieldType->getKind() == TypeReflection::Kind::Vector) {
            numElements = fieldType->getElementCount();
            assert(numElements >= 2 && numElements <= 4);
            scalarType = GetPetalScalarType(fieldType->getElementTypeLayout());
        } else {
            PETAL_CHECK_COND(
                fieldType->getKind() != TypeReflection::Kind::Scalar,
                Result::PETAL_SHADER_REFLECTION_INVALID_TYPE,
                m_logger,
                "{} wasn't a scalar or vector (fragment shader output field)", fieldType->getKind()
            );
            scalarType = GetPetalScalarType(fieldType);
        }

        PETAL_CHECK_OPTIONAL(
            scalarType,
            m_logger,
            "Failed to get scalar type of fragment output {} (type {})", (field->getName() ? field->getName() : ""), (fieldType->getName() ? fieldType->getName() : "")
        );

        IntermediateShaderResource::FragmentOutput output = {
            .Name = field->getName() ? field->getName() : "",
            .NumElements = numElements,
            .Type = scalarType.Value()
        };

        if (output.Name.empty()) shader.FragmentShader->ContainsUnnamedOutputs = true;
        shader.FragmentShader->Outputs.push_back(output);

        return Result::SUCCESS;
    }

    void ShaderSubsystem::ReflectGetAllFieldsRecursive(
        VariableLayoutReflection *variableLayout,
        std::vector<VariableLayoutReflection *> &fieldsOut
    ) {
        TypeReflection::Kind kind = variableLayout->getTypeLayout()->getKind();
        if (kind == TypeReflection::Kind::Struct) {
            // struct - recursively iterate
            if (variableLayout->getTypeLayout()->getFieldCount() == 0) {
                m_logger->Warn("Struct {} with no fields in shader", variableLayout->getName());
            }

            for (glm::u32 fieldIndex = 0; fieldIndex < variableLayout->getTypeLayout()->getFieldCount(); fieldIndex++) {
                VariableLayoutReflection *field = variableLayout->getTypeLayout()->getFieldByIndex(fieldIndex);
                ReflectGetAllFieldsRecursive(field, fieldsOut);
            }
        } else {
            // regular field - push
            fieldsOut.push_back(variableLayout);
        }
    }
} // Petal
