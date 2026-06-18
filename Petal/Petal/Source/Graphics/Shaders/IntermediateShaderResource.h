#pragma once
#include "ShaderType.h"
#include "slang.h"
#include "Common.h"
#include "Graphics/VertexType.h"
#include "Graphics/Resources/ShaderResource.h"

namespace Petal {
    struct RenderTarget;

    // Represents a compiled shader that has yet to be tied to a specific graphics API
    struct IntermediateShaderResource {
        enum class ScalarType {
            FLOAT, INT, UNSIGNED_INT
        };

        struct ShaderStage {
            std::string EntryFunctionName;
            glm::u32 EntryPointIndex;
            Slang::ComPtr<slang::IBlob> SPIRV;
        };

        struct FragmentOutput {
            std::string Name;
            glm::u32 NumElements;
            ScalarType Type;
        };

        struct FragmentShaderInfo {
            std::vector<FragmentOutput> Outputs;
            // True if any of the FragmentOutput names are blank
            bool ContainsUnnamedOutputs;

            // Check if the fragment output matches the render target color attachments
            // If logger is not null, it will be used to log the mismatch
            bool MatchesRenderTarget(
                const RenderTarget &renderTarget,
                OptionalRef<const std::shared_ptr<Logger>> logger = Result::PETAL_OPTIONAL_EMPTY
            ) const;
        };

        std::vector<ShaderResource> Resources;
        std::unordered_map<ShaderType, ShaderStage> ShaderTypes;

        // This will be set unless no vertex shader exists in the shader
        // it contains information on vertex size and attributes
        // The vertex is assumed to be the sum of all VARYING_INPUT parameters to the vertex shader
        Optional<VertexType> VertexType = Result::PETAL_OPTIONAL_EMPTY;

        // List of fragment shader outputs
        // This will be set unless no fragment shader exists in the shader
        Optional<FragmentShaderInfo> FragmentShader = Result::PETAL_OPTIONAL_EMPTY;

        // Check if the VkFormat matches the scalar type and num elements
        static bool IsValidFormat(VkFormat format, ScalarType type, glm::u32 numElements);
    };
} // Petal

PETAL_MAKE_ENUM_FORMATTABLE(Petal::IntermediateShaderResource::ScalarType);
