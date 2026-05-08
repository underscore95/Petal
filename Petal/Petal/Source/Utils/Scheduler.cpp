#include "Scheduler.h"

#include "Engine.h"

namespace Petal {
    Scheduler::Scheduler(const Engine &engine) {
        m_logger = engine.GetLoggerSystem().GetLogger(LoggerSystem::SCHEDULER_LOGGER);
    }

    Scheduler::~Scheduler() {
        size_t scheduledItems = m_framePendingDeletions.size() + m_framePendingFunctions.size();
        if (scheduledItems > 0) {
            m_logger->Warn("Destroyed Scheduler with {} scheduled items remaining. Shared pointers will be destroyed now. Functions will not be executed.", scheduledItems);
        }
    }

    void Scheduler::ScheduleFrames(const std::function<void()> &function, glm::u32 numFrames) {
        if (numFrames == 0) {
            m_logger->Warn("Scheduled function in 0 frames");
            function();
            return;
        }

        m_framePendingFunctions.emplace_back(function, numFrames);
    }

    void Scheduler::ScheduleDeletionFrames(
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

            pending[i].TimeRemaining--;
            i++;
        }
    }

    void Scheduler::Render() {
        Decrement(m_framePendingDeletions, {});
        Decrement<std::function<void()>, glm::u32>(m_framePendingFunctions, [](std::function<void()> &function) { function(); });
    }
} // Petal
