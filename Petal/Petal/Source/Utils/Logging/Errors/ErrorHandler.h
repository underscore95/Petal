#pragma once

#include "Result.h"
#include "Logging/Logger.h"

namespace Petal {
    // If condition is true, the current function will return error code and the logger will print a formatted error message
#define PETAL_CHECK_COND(condition, errorCode, loggerRef, message, ...) \
    do { \
        static_assert(typeid(errorCode) == typeid(Result)); \
        static_assert(typeid(loggerRef) == typeid(std::shared_ptr<Logger>)); \
        static_assert(std::string_view(#condition) != "result == Result::SUCCESS" && "Unexpected condition, did you accidentally invert it?"); \
        static_assert(std::string_view(#condition) != "res == Result::SUCCESS" && "Unexpected condition, did you accidentally invert it?"); \
        static_assert(std::string_view(#condition) != "res == VK_SUCCESS" && "Unexpected condition, did you accidentally invert it?"); \
        static_assert(std::string_view(#condition) != "result == VK_SUCCESS" && "Unexpected condition, did you accidentally invert it?"); \
        const bool& __conditionStored = condition; \
        if (__conditionStored) [[unlikely]] { \
            std::string errorMessageFormatted = std::format(message __VA_OPT__(,) __VA_ARGS__);\
            loggerRef->Error("[Error Code: {}] {}", errorCode, errorMessageFormatted); \
            assert(errorCode != Result::SUCCESS && "Why is the error code SUCCESS?"); \
            return errorCode; \
        } \
    } while (0)

    // Only use if there is no sensible error message
#define PETAL_CHECK_COND_SILENT(condition, errorCode) \
    do { \
        static_assert(typeid(errorCode) == typeid(Result)); \
        static_assert(std::string_view(#condition) != "result == Result::SUCCESS" && "Unexpected condition, did you accidentally invert it?"); \
        static_assert(std::string_view(#condition) != "res == Result::SUCCESS" && "Unexpected condition, did you accidentally invert it?"); \
        static_assert(std::string_view(#condition) != "res == VK_SUCCESS" && "Unexpected condition, did you accidentally invert it?"); \
        static_assert(std::string_view(#condition) != "result == VK_SUCCESS" && "Unexpected condition, did you accidentally invert it?"); \
        const bool& __conditionStored = condition; \
        if (__conditionStored) [[unlikely]] { \
            assert(errorCode != Result::SUCCESS && "Why is the error code SUCCESS?"); \
            return errorCode; \
        } \
} while (0)
} // Petal
