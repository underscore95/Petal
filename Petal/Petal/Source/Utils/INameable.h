#pragma once

#include "pch.h"

namespace Petal {
    class INameable {
    public:
        virtual ~INameable() = default;

    public:
        virtual const std::string &GetName() const = 0;

    public:
        // Convert from a vector of nameables to a vector of names (strings)
        template<typename T>
            requires std::derived_from<T, INameable>
        static std::vector<std::string> ToNameVector(const std::vector<std::shared_ptr<T> > &vector) {
            std::vector<std::string> names(vector.size());
            for (size_t i = 0; i < vector.size(); i++) {
                names[i] = vector[i] ? vector[i]->GetName() : "nullptr";
            }
            return names;
        }
    };
}
