#pragma once

#include "Common.h"
#include "ShaderType.h"

namespace Petal {
    struct ShaderAsset {
        // Single source file; single shader type
        ShaderAsset(
            const std::filesystem::path &path,
            ShaderType shaderType
        ) {
            SourceFiles[shaderType] = path;
        }

        // Multiple source files; each source file contains entry points for a specific shader type
        ShaderAsset(
            const std::filesystem::path &path,
            const std::vector<ShaderType> &shaderTypes
        ) {
            for (ShaderType type : shaderTypes) {
                SourceFiles[type] = path;
            }
        }

        // Single source file that contains multiple entry points for different shader types
        explicit ShaderAsset(
            const std::unordered_map<ShaderType, std::filesystem::path> &files
        ) : SourceFiles(files) {
        }

        std::unordered_map<ShaderType, std::filesystem::path> SourceFiles;
    };
} // Petal
