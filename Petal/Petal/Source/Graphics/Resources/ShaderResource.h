#pragma once

#include "ResourceType.h"
#include "Common.h"
#include "Graphics/Shaders/ShaderType.h"

namespace Petal {
    struct ShaderResource {
        std::string Name;
        ResourceType Type;
        glm::u32 BindingIndex;
        glm::u32 BindingSet;
        std::vector<ShaderType> Stages;
        bool IsArray;
        // If the resource is an array, and the size of the array is defined in the shader, how big is it?
        // If this is 0, the resource either is not an array or is dynamic
        glm::u32 StaticArraySize;

        std::string ToString();

        bool IsStaticArray() const { return IsArray && StaticArraySize != 0; }
    };
} // Petal
