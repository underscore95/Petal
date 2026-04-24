#pragma once

#include "Result.h"
#include "Logging/Logger.h"

namespace Petal {
    // If condition is true, the current function will return error code and the logger will print a formatted error message
#define PETAL_CHECK_COND(condition, errorCode, loggerRef, message, ...) \
    static_assert(typeid(errorCode) == typeid(Result)); \
    static_assert(typeid(loggerRef) == typeid(std::shared_ptr<Logger>)); \
    do { \
        if (condition) [[unlikely]] { \
            std::string errorMessageFormatted = std::format(message __VA_OPT__(,) __VA_ARGS__);\
            loggerRef->Error("[Error Code: {}] {}", (int)errorCode, errorMessageFormatted); \
            return errorCode; \
        } \
    } while (0)

    // Only use if there is no sensible error message
#define PETAL_CHECK_COND_SILENT(condition, errorCode) \
    static_assert(typeid(errorCode) == typeid(Result)); \
    do { \
        if (condition) [[unlikely]] { \
            return errorCode; \
        } \
    } while (0)

} // Petal
