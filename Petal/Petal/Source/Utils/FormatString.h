#pragma once

#include "pch.h"

// Source - https://stackoverflow.com/a/59914918
// Posted by vitaut, modified by community. See post 'Timeline' for change history
// Retrieved 2026-04-11, License - CC BY-SA 4.0

// Allows a type to be used in std::format

// Example usage:
// PETAL_MAKE_FORMATTABLE_BODY(Petal::LogLevel, instance, {
//     return formatter<std::string>::format(
//         std::format("{}", Petal::LogLevelEnum::Names[static_cast<int>(instance)]),
//         ctx
//     );
// });

#define PETAL_MAKE_FORMATTABLE_BODY(type, varName, BODY) \
template<> \
struct std::formatter<type> : std::formatter<std::string> { \
    auto format(type varName, format_context& ctx) const BODY \
};

// Example usage:
// PETAL_MAKE_FORMATTABLE(Petal::LogLevel, instance,
//     std::format("{}", Petal::LogLevelEnum::Names[static_cast<int>(instance)])
// )

#define PETAL_MAKE_FORMATTABLE(type, varName, EXPR) \
template<> \
struct std::formatter<type> : std::formatter<std::string> { \
    auto format(const type& varName, format_context& ctx) const { \
        return std::formatter<std::string>::format((EXPR), ctx); \
    } \
};

// Allows an enum to be used in std::format
// Uses magic_enum to get value name
// Example usage:
// PETAL_MAKE_ENUM_FORMATTABLE(slang::TypeReflection::Kind)
#define PETAL_MAKE_ENUM_FORMATTABLE(type) \
template<> \
struct std::formatter<type> : std::formatter<std::string> { \
    auto format(type value, format_context& ctx) const { \
        if (auto name = magic_enum::enum_name(value); !name.empty()) { \
            return std::formatter<std::string>::format(std::string(name), ctx); \
        } \
        return std::formatter<std::string>::format( \
            std::format("Unknown ({} {})", #type, static_cast<int>(value)), ctx); \
    } \
};

// Common types
PETAL_MAKE_FORMATTABLE(
    std::filesystem::path, path,
    std::format("{}", path.string())
);