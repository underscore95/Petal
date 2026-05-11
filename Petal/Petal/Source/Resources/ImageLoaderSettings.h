#pragma once

namespace Petal {
    struct ImageLoaderSettings {
        bool FlipVertically = false;

        void Validate();
    };
} // Petal
