#pragma once

#include "FormatString.h"

// std::vector format
template<typename T>
struct std::formatter<std::vector<T>> : std::formatter<std::string> {
    auto format(const std::vector<T>& vec, std::format_context& ctx) const {
        std::string s = "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            s += std::format("{}", vec[i]);
            if (i + 1 < vec.size()) s += ", ";
        }
        s += "]";
        return std::formatter<std::string>::format(s, ctx);
    }
};

// std::unordered_map format
template <typename K, typename V, typename CharT>
struct std::formatter<std::unordered_map<K, V>, CharT> {
    std::formatter<K, CharT> key_fmt;
    std::formatter<V, CharT> val_fmt;

    constexpr auto parse(std::basic_format_parse_context<CharT>& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const std::unordered_map<K, V>& map, FormatContext& ctx) const {
        auto out = ctx.out();
        *out++ = '{';

        size_t i = 0;
        for (const auto& [k, v] : map) {
            out = key_fmt.format(k, ctx);
            *out++ = ':';
            *out++ = ' ';
            out = val_fmt.format(v, ctx);

            if (++i < map.size()) {
                *out++ = ',';
                *out++ = ' ';
            }
        }

        *out++ = '}';
        return out;
    }
};