#pragma once

#include "Common.h"

namespace Petal {
    struct RendererSettings {
        std::string CameraBufferName;

        Result Validate(const std::shared_ptr<Logger> &logger) const;
    };
} // Petal
