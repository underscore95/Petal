#pragma once

#include "Common.h"

namespace Petal {
    struct RendererSettings {
        std::string VertexBufferShaderName;
        std::string IndexBufferShaderName;

        Result Validate(const std::shared_ptr<Logger> &logger);
    };
} // Petal
