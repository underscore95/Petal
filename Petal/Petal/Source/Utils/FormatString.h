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
// std::filesystem::path
PETAL_MAKE_FORMATTABLE(
    std::filesystem::path, path,
    std::format("{}", path.string())
);

// glm::mat4x4
template<>
struct std::formatter<glm::mat4>
{
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const glm::mat4& m, FormatContext& ctx) const
    {
        return std::format_to(
            ctx.out(),
            "{{{{{:.5f}, {:.5f}, {:.5f}, {:.5f}}},\n"
            " {{{:.5f}, {:.5f}, {:.5f}, {:.5f}}},\n"
            " {{{:.5f}, {:.5f}, {:.5f}, {:.5f}}},\n"
            " {{{:.5f}, {:.5f}, {:.5f}, {:.5f}}}}}",
            m[0][0], m[1][0], m[2][0], m[3][0],
            m[0][1], m[1][1], m[2][1], m[3][1],
            m[0][2], m[1][2], m[2][2], m[3][2],
            m[0][3], m[1][3], m[2][3], m[3][3]);
    }
};