#pragma once

#include "pch.h"

namespace Petal {
    template<typename T>
    using Ref = std::shared_ptr<T>;

    template<typename T>
    using WeakRef = std::weak_ptr<T>;
} // Petal
