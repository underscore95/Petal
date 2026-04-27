#pragma once
#include "ShaderType.h"
#include "slang.h"
#include "Common.h"

namespace Petal {
    struct IntermediateShaderResource {
        Slang::ComPtr<slang::IBlob> SPIRV;
    };
} // Petal
