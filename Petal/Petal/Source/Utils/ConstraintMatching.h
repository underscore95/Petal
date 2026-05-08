#pragma once

#include "Common.h"

namespace Petal {
    class ConstraintMatching {
    public:
        ConstraintMatching() = delete;

    public:
        // Solve constraint matching problems
        // input is a 2d array where each inner array is a list of values which meet the requirement
        // output is the list of values to use
        // there is no guarantee that all values are used, however output vector will be same size as input vector if a solution is possible
        // e.g.
        // { {0, 1, 2}, {1, 2}, {1}, {1, 3}}
        // index 0 can be 0, 1, or 2
        // index 1 can be 1 or 2
        // index 2 can be 1
        // index 3 can be 1 or 3
        // so correct output is {0, 2, 1, 3}
        static std::vector<glm::u32> ConstructSolution(const std::vector<std::vector<glm::u32> > &input);
    };
} // Petal
