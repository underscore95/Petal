#pragma once

#include "pch.h"

namespace Petal {
    class Scheduler {
        friend class Engine;

    public:
        enum class CancellableTaskType {
            FRAMES_SYNC
        };

        template<CancellableTaskType Type>
        using CancellableTaskId = glm::u32;

        typedef CancellableTaskId<CancellableTaskType::FRAMES_SYNC> SyncTaskId;

        template<typename ValueType, typename TimeUnit>
        struct Pending {
            ValueType Value;
            TimeUnit TimeRemaining;

            bool operator<(const Pending &other) const {
                return TimeRemaining < other.TimeRemaining;
            }
        };

    public:
        explicit Scheduler(
            const std::shared_ptr<Logger> &logger
        );

        ~Scheduler();

    public:
        // Call function after number of frames
        void ScheduleFramesSync(const std::function<void()> &function, glm::u32 numFrames);

        SyncTaskId ScheduleFramesSyncCancellable(const std::function<void()> &function, glm::u32 numFrames);

        // Destroy the pointer after number of frames
        // Note that if other shared pointers still exist, the contained object won't be deleted
        void ScheduleDeletionFramesSync(const std::shared_ptr<void> &ptr, glm::u32 numFrames);

        // Cancel a task, returns true if the task was cancelled, false if the task didn't exist (meaning it probably already executed)
        template<CancellableTaskType Type>
        bool Cancel(CancellableTaskId<Type>) = delete;

        template<>
        bool Cancel<CancellableTaskType::FRAMES_SYNC>(SyncTaskId id);

    private:
        // TODO: this could be multithreaded
        void Render();

    private:
        std::shared_ptr<Logger> m_logger;
        std::vector<Pending<std::shared_ptr<void>, glm::u32> > m_framePendingDeletions;
        std::vector<Pending<std::function<void()>, glm::u32> > m_framePendingFunctions;
        std::unordered_map<SyncTaskId, Pending<std::function<void()>, glm::u32> > m_framePendingFunctionsCancellable;
        SyncTaskId m_nextSyncTaskId; // todo atomic
    };
} // Petal
