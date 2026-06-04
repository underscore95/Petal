#pragma once

#include "pch.h"

namespace Petal {
    class INameable {
    public:
        virtual ~INameable() = default;

    public:
        virtual const std::string &GetName() const = 0;
    };
}
