#ifndef PETAL_GUARD_ALIGNMENT
#define PETAL_GUARD_ALIGNMENT

// PADDING
#ifdef __cplusplus

#include <glm/glm.hpp>

#ifndef CONCAT
#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)
#endif

#define UINT_PADDING_1 \
    glm::u32 CONCAT(_padding_, __COUNTER__)

#define UINT_PADDING_2 \
    UINT_PADDING_1; \
    UINT_PADDING_1

#define UINT_PADDING_3 \
    UINT_PADDING_2; \
    UINT_PADDING_1

#define UINT_PADDING_4 \
    UINT_PADDING_3; \
    UINT_PADDING_1

#else

#define UINT_PADDING_1
#define UINT_PADDING_2
#define UINT_PADDING_3
#define UINT_PADDING_4

#endif

#endif
