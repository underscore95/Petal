#pragma once

#include "Common.h"
#include "ShaderInfo.h"
#include "ShaderType.h"
#include "Graphics/VertexType.h"

namespace Petal {
    struct ShaderAsset {
        std::filesystem::path Source;
        std::unordered_map<ShaderType, ShaderInfo> Shaders;
    };
} // Petal
