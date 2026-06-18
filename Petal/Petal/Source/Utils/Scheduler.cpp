#include "Scheduler.h"

namespace Petal {
    Scheduler::Scheduler(
        const std::shared_ptr<Logger> &logger
    )
        : m_logger(logger) {
    }

    Scheduler::~Scheduler() {
        size_t scheduledItems = m_framePendingDeletions.size() + m_framePendingFunctions.size() + m_framePendingFunctionsCancellable.size();
        if (scheduledItems > 0) {
            m_logger->Warn("Destroyed Scheduler with {} scheduled items remaining. Shared pointers will be destroyed now. Functions will not be executed.", scheduledItems);
        }
    }

    void Scheduler::ScheduleFramesSync(const std::function<void()> &function, glm::u32 numFrames) {
        if (numFrames == 0) {
            m_logger->Warn("Scheduled function in 0 frames");
            function();
            return;
        }

        m_framePendingFunctions.emplace_back(function, numFrames);
    }

    Scheduler::SyncTaskId Scheduler::ScheduleFramesSyncCancellable(const std::function<void()> &function, glm::u32 numFrames) {
        if (numFrames == 0) {
            m_logger->Warn("Scheduled function in 0 frames");
            function();
            return m_nextSyncTaskId++;
        }

        SyncTaskId taskId = m_nextSyncTaskId++;
        m_framePendingFunctionsCancellable.emplace(taskId, Pending<std::function<void()>, glm::u32>{.Value = function, .TimeRemaining = numFrames});

        return taskId;
    }

    void Scheduler::ScheduleDeletionFramesSync(
        const std::shared_ptr<void> &ptr,
        glm::u32 numFrames
    ) {
        if (numFrames == 0) {
            m_logger->Warn("Scheduled pointer for deletion in 0 frames");
            return;
        }
        m_framePendingDeletions.emplace_back(ptr, numFrames);
    }

    template<typename ValueType, typename TimeUnit>
    void Decrement(
        std::vector<Scheduler::Pending<ValueType, TimeUnit> > &pending,
        const std::function<void(ValueType &)> &action
    ) {
        for (size_t i = 0; i < pending.size();) {
            if (pending[i].TimeRemaining == 0) {
                action(pending[i].Value);
                pending[i] = pending.back();
                pending.pop_back();
                continue;
            }

            --pending[i].TimeRemaining;
            i++;
        }
    }

    template<typename ValueType, typename TimeUnit>
    void DecrementMap(
        std::unordered_map<Scheduler::SyncTaskId, Scheduler::Pending<ValueType, TimeUnit> > &pending,
        const std::function<void(ValueType &)> &action
    ) {
        for (auto it = pending.begin(); it != pending.end();) {
            auto &task = it->second;

            --task.TimeRemaining;

            if (task.TimeRemaining == 0) {
                action(task.Value);
                it = pending.erase(it);
            } else {
                ++it;
            }
        }
    }

    template<>
    bool Scheduler::Cancel<Scheduler::CancellableTaskType::FRAMES_SYNC>(SyncTaskId id) {
        return m_framePendingFunctionsCancellable.erase(id) != 0;
    }

    void Scheduler::Render() {
        Decrement(m_framePendingDeletions, {});
        Decrement<std::function<void()>, glm::u32>(m_framePendingFunctions, [](std::function<void()> &function) { function(); });
        DecrementMap<std::function<void()>, glm::u32>(m_framePendingFunctionsCancellable, [](std::function<void()> &function) { function(); });
    }
} // Petal
