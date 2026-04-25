#include "Timer.h"

namespace Petal {
    Timer::Timer() {
        Restart();
    }

    void Timer::Restart() {
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    float Timer::SecondsSinceStart() const {
        const auto duration = std::chrono::high_resolution_clock::now() - m_startTime;
        return static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()) / 1000000000.0f;
    }

    float Timer::MillisSinceStart() const {
        return SecondsSinceStart() * 1000.0f;
    }
} // Petal