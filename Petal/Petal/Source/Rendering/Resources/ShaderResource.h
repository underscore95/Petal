#pragma once

#include "ResourceType.h"
#include "Common.h"
#include "Rendering/Shaders/ShaderType.h"

namespace Petal {
    struct ShaderResource {
        std::string Name;
        ResourceType Type;
        // Size of the contained type, this may be 0 for example if Sampler2D
        std::size_t Size;
        glm::u32 BindingIndex;
        glm::u32 BindingSet;
        std::vector<ShaderType> Stages;

        std::string ToString();
    };
} // Petal