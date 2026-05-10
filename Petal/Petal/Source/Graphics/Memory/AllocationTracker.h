#pragma once

#include "Common.h"

namespace Petal {
    struct Allocation {
        glm::u32 Location;
        glm::u32 Size;
    };

    class AllocationTracker {
    public:
        explicit AllocationTracker(
            const std::shared_ptr<Logger> &logger,
            glm::u32 size
        );

        ~AllocationTracker();

    public:
        Optional<Allocation> Allocate(glm::u32 size);

        void Free(Allocation allocation);

    private:
        std::shared_ptr<Logger> m_logger;
        std::vector<Allocation> m_allocations; // location -> size
       const glm::u32 m_size;
        // num bytes free
        glm::u32 m_freeSpace;
        // if false, all allocations are definitely continuous without gaps starting at byte 0, if true there may or may not be a gap
        bool m_hasAnyGapsProbably = false;
    };
} // Petal
