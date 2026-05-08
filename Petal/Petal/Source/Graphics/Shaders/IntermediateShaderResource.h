#pragma once
#include "ShaderType.h"
#include "slang.h"
#include "Common.h"
#include "Graphics/Resources/ShaderResource.h"

namespace Petal {
    // Represents a compiled shader that has yet to be tied to a specific graphics API
    struct IntermediateShaderResource {
        struct ShaderStage {
            std::string EntryFunctionName;
            glm::u32 EntryPointIndex;
            Slang::ComPtr<slang::IBlob> SPIRV;
        };

        std::vector<ShaderResource> Resources;
        std::unordered_map<ShaderType, ShaderStage> ShaderTypes;
    };
} // Petal
