#pragma once

#include "Common.h"
#include "ShaderInfo.h"
#include "ShaderType.h"

namespace Petal {
    struct ShaderAsset {
        // Single source file; single shader type
        ShaderAsset(
            const std::filesystem::path &path,
            ShaderType shaderType,
            ShaderInfo shaderInfo
        ) : Source(path) {
            Shaders[shaderType] = shaderInfo;
        }

        // Single source file; multiple shader types
        ShaderAsset(
            const std::filesystem::path &path,
            const std::unordered_map<ShaderType, ShaderInfo> &shaders
        ) : Source(path), Shaders(shaders) {
        }

    public:
        std::filesystem::path Source;
        std::unordered_map<ShaderType, ShaderInfo> Shaders;
    };
} // Petal
