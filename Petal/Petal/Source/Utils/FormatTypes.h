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

PETAL_MAKE_FORMATTABLE(
    std::filesystem::path, path,
    std::format("{}", path.string())
);