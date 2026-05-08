#pragma once

#include "pch.h"

namespace Petal {
    class Scheduler {
        friend class Engine;

    public:
        template<typename ValueType, typename TimeUnit>
        struct Pending {
            ValueType Value;
            TimeUnit TimeRemaining;

            bool operator<(const Pending &other) const {
                return TimeRemaining < other.TimeRemaining;
            }
        };

    public:
        explicit Scheduler(const Engine &engine);

        ~Scheduler();

    public:
        // Call function after number of frames
        void ScheduleFrames(const std::function<void()> &function, glm::u32 numFrames);

        // Destroy the pointer after number of frames
        // Note that if other shared pointers still exist, the contained object won't be deleted
        void ScheduleDeletionFrames(const std::shared_ptr<void> &ptr, glm::u32 numFrames);

    private:
        // TODO: this could be multithreaded
        void Render();

    private:
        std::shared_ptr<Logger> m_logger;
        std::vector<Pending<std::shared_ptr<void>, glm::u32> > m_framePendingDeletions;
        std::vector<Pending<std::function<void()>, glm::u32> > m_framePendingFunctions;
    };
} // Petal
