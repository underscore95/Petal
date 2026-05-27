#pragma once

#include "Common.h"

namespace Petal {
    struct RendererSettings {
        Result Validate(const std::shared_ptr<Logger> &logger);
    };
} // Petal
