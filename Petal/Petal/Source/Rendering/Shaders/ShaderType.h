#pragma once

namespace Petal {
    enum class ShaderType {
        VERTEX,
        FRAGMENT
    };

    // Entry point functions should be decorated with e.g. [shader("vertex")]
    constexpr std::string GetShaderEntryPoint(ShaderType type) {
        switch (type) {
            case ShaderType::VERTEX: return "vertexMain";
            case ShaderType::FRAGMENT: return "fragmentMain";
        }
        assert(false);
        return "";
    }
} // Petal
