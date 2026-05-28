#pragma once

#include "Common.h"

namespace Petal {
    struct VertexType {
        struct Attribute {
            std::string Name;
            glm::u32 Size;
            VkFormat Format;
        };

        std::vector<Attribute> Attributes;
        // Summed size of attributes
        glm::u32 Size = 0;
    };
}
