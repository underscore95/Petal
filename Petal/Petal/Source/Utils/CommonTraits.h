#pragma once

#include "pch.h"

namespace Petal {
    template<typename T>
        struct IsPointer : std::is_pointer<T> {};

    template<typename T>
    struct IsReference : std::is_reference<T> {};

    template<typename T>
    struct IsUniquePtr : std::false_type {};

    template<typename U>
    struct IsUniquePtr<std::unique_ptr<U>> : std::true_type {};


} // Petal