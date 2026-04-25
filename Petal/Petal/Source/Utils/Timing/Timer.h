#pragma once

#include "Common.h"

namespace Petal {
    class Timer {
    public:
        Timer(); // Starts automatically

    public:
        void Restart();

        [[nodiscard]] float SecondsSinceStart() const;

        [[nodiscard]] float MillisSinceStart() const;

    private:
        std::chrono::high_resolution_clock::time_point m_startTime;
    };
} // Petal
