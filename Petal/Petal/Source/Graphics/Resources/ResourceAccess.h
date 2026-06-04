#pragma once

#include "Common.h"

namespace Petal {
    enum class ResourceAccess {
        // Make sure to fill out EnumData in the .cpp
        READ,
        WRITE,
        READ_WRITE,
        COUNT,
    };

    class ResourceAccesses {
        ResourceAccesses() = delete;

    public:
        struct EnumData {
            const char *Name;
            bool IsRead;
            bool IsWrite;

            explicit EnumData(
                const char *name,
                bool isRead,
                bool isWrite
            )
                : Name(name),
                  IsRead(isRead),
                  IsWrite(isWrite) {
            }
        };

    public:
        static const EnumData &GetData(ResourceAccess resourceAccess);

    private:
        static const std::array<EnumData, static_cast<size_t>(ResourceAccess::COUNT)> Data;
    };
}

PETAL_MAKE_ENUM_FORMATTABLE(Petal::ResourceAccess);
