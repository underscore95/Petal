#pragma once
#include "ShaderType.h"
#include "slang.h"
#include "Common.h"
#include "Rendering/Resources/ShaderResource.h"

namespace Petal {
    // Represents a compiled shader that has yet to be tied to a specific graphics API
    struct IntermediateShaderResource {
        Slang::ComPtr<slang::IBlob> SPIRV;
        std::vector<ShaderResource> Resources;
    };
} // Petal
