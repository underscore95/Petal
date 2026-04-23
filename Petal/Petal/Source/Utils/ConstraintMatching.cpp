#include "ConstraintMatching.h"

namespace Petal {
    bool Solve(
        const std::vector<std::vector<glm::u32> > &input,
        glm::u32 i,
        std::vector<glm::u32> &result, std::unordered_set<glm::u32> &used
    ) {
        if (i == input.size()) return true;

        for (int v : input[i]) {
            if (used.contains(v)) continue;

            result[i] = v;
            used.insert(v);

            if (Solve(input, i + 1, result, used)) return true;

            used.erase(v);
        }
        return false;
    }

    std::vector<glm::u32> ConstraintMatching::ConstructSolution(
        const std::vector<std::vector<glm::u32> > &input
    ) {
        std::vector<glm::u32> result(input.size());
        std::unordered_set<glm::u32> used;
        Solve(input, 0, result, used);
        return result;
    }
} // Petal
