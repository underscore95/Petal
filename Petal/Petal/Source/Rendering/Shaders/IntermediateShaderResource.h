#pragma once
#include "ShaderType.h"
#include "slang.h"
#include "Common.h"

namespace Petal {
    struct IntermediateShaderResource {
        std::unordered_map<ShaderType, Slang::ComPtr<slang::IModule> > SlangModules;
        std::unordered_map<ShaderType, Slang::ComPtr<slang::IComponentType> > LinkedPrograms;
        std::unordered_map<ShaderType, Slang::ComPtr<slang::IBlob> > SPIRV;
    };
} // Petal
