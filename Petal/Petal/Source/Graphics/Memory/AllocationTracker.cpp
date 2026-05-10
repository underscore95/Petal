#include "AllocationTracker.h"

namespace Petal {
    AllocationTracker::AllocationTracker(
        const std::shared_ptr<Logger> &logger,
        glm::u32 size
    )
        : m_logger(logger),
          m_size(size),
          m_freeSpace(size) {
    }

    AllocationTracker::~AllocationTracker() {
        if (!m_allocations.empty()) {
            m_logger->Warn("Destroyed AllocationTracker with {} unallocated blocks", m_allocations.size());
        }
    }

    Optional<Allocation> AllocationTracker::Allocate(
        glm::u32 size
    ) {
        if (size == 0) {
            m_logger->Error("[AllocationTracker] Attempted to allocate 0 bytes");
            return Result::PETAL_ALLOCATION_FAILED;
        }

        if (m_hasAnyGapsProbably) {
            if (m_freeSpace <= size) {
                // Allocated memory is not continuous so even if free space is exactly size, the allocated memory will be in the middle preventing the allocation
                m_logger->Error("[AllocationTracker] Not enough space remaining");
                return Result::PETAL_ALLOCATION_FAILED;
            }

            // try and allocate in a gap
            for (glm::u32 i = 0; i < m_allocations.size() - 1; i++) {
                Allocation allocation = m_allocations[i];
                Allocation nextAllocation = m_allocations[i + 1];
                glm::u32 gap = nextAllocation.Location - (allocation.Location + allocation.Size);
                if (gap < size) continue;

                Allocation newAllocation = {(allocation.Location + allocation.Size), size};
                m_allocations.insert(m_allocations.begin() + i, newAllocation);
                m_freeSpace -= size;
                return newAllocation;
            }
        }

        // try and allocate at the end
        glm::u32 location = m_size - m_freeSpace;
        if (location + size > m_size) {
            m_logger->Error("[AllocationTracker] Not enough space remaining");
            return Result::PETAL_ALLOCATION_FAILED;
        }

        Allocation allocation = {location, size};
        m_allocations.push_back(allocation);
        m_freeSpace -= size;
        return allocation;
    }

    void AllocationTracker::Free(Allocation allocation) {
        // assume earlier allocations are more likely to be freed first
        for (glm::u32 i = 0; i < m_allocations.size(); i++) {
            Allocation existingAllocation = m_allocations[i];
            if (existingAllocation.Location != allocation.Location) continue;
            if (existingAllocation.Size != allocation.Size) {
                m_logger->Warn(
                    "[AllocationTracker] When freeing allocation at {}, the requested free was only {} bytes but the actual allocation was {} bytes. {} bytes have been freed.",
                    allocation.Location, allocation.Size, existingAllocation.Size, existingAllocation.Size
                );
            }
            m_freeSpace -= existingAllocation.Size;
            if (i != m_allocations.size() + 1) {
                // freeing in the middle
                m_hasAnyGapsProbably = true;
            }

            m_allocations.erase(m_allocations.begin() + i);
            return;
        }

        m_logger->Warn(
            "[AllocationTracker] Attempted to free allocation at {} with size {} but there was no allocation there",
            allocation.Location, allocation.Size
        );
    }
} // Petal
