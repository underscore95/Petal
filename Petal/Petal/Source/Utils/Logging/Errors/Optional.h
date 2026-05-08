#pragma once

#include "Result.h"

namespace Petal {
    template<typename T>
    class Optional {
        static_assert(!IsResult<T>::value, "Optional<Result> is forbidden because it creates constructor ambiguity.");
        static_assert(!IsUniquePtr<T>::value, "Optional<std::unique_ptr<T>> is forbidden. Use AllocatedOptional<T> instead.");

    public:
        Optional(const T &value)
            : m_value(value),
              m_present(true),
              m_result(Result::SUCCESS) {
        }

        Optional(Result result)
            : m_present(false),
              m_result(result) {
            if (result == Result::SUCCESS) [[unlikely]] {
#ifndef NDEBUG
                __debugbreak();
#endif
            }
        }

        Optional(const Optional &other) = delete;

        Optional(Optional &&other) noexcept
            : m_value(std::move(other.m_value)),
              m_present(other.m_present),
              m_result(other.m_result) {
        }

        Optional &operator=(const Optional &other) = delete;

        Optional &operator=(Optional &&other) noexcept {
            if (this == &other)
                return *this;

            m_value = std::move(other.m_value);
            m_present = other.m_present;
            m_result = other.m_result;

            other.m_present = false;
            other.m_result = Result::PETAL_OPTIONAL_MOVED_OUT;

            return *this;
        }

        ~Optional() = default;

        static constexpr Optional Empty() {
            return Result::PETAL_OPTIONAL_EMPTY;
        }

    public:
        Result GetResult() const { return m_result; }
        T *Value() { return m_present ? &m_value : nullptr; }
        bool HasValue() const { return m_present; }
        bool IsEmpty() const { return !HasValue(); }

        T *operator->() {
            assert(HasValue());
            return m_value.get();
        }

    private:
        T m_value;
        bool m_present;
        Result m_result;
    };

    template<typename T>
    class AllocatedOptional {
        static_assert(!IsPointer<T>::value, "AllocatedOptional<T*> is forbidden. Use Optionalstd::shared_ptr<T> instead.");
        static_assert(!IsReference<T>::value, "AllocatedOptional<T&> is forbidden. Use Optionalstd::shared_ptr<T> instead.");
        static_assert(!IsResult<T>::value, "AllocatedOptional<Result> is forbidden because it creates constructor ambiguity.");
        static_assert(!IsUniquePtr<T>::value, "AllocatedOptional<std::unique_ptr<T>> is forbidden because it already stores a unique_ptr.");

    public:
        AllocatedOptional(std::unique_ptr<T> value)
            : m_value(std::move(value)),
              m_present(true),
              m_result(Result::SUCCESS) {
        }

        AllocatedOptional(const Result &result)
            : m_present(false),
              m_result(result) {
            if (result == Result::SUCCESS) [[unlikely]] {
#ifndef NDEBUG
                __debugbreak();
#endif
            }
        }

        AllocatedOptional(const AllocatedOptional &other) = delete;

        AllocatedOptional(AllocatedOptional &&other) noexcept
            : m_value(std::move(other.m_value)),
              m_present(other.m_present),
              m_result(other.m_result) {
        }

        AllocatedOptional &operator=(const AllocatedOptional &other) = delete;

        AllocatedOptional &operator=(AllocatedOptional &&other) noexcept {
            if (this == &other)
                return *this;
            m_value = std::move(other.m_value);
            m_present = other.m_present;
            m_result = other.m_result;
            other.m_value = nullptr;
            other.m_present = false;
            other.m_result = Result::PETAL_OPTIONAL_MOVED_OUT;
            return *this;
        }

        // Set the value in the optional to empty and return the value
        std::unique_ptr<T> Release() {
            assert(HasValue());
            m_result = Result::PETAL_OPTIONAL_RELEASED;
            m_present = false;
            return std::move(m_value);
        }

        template<typename... Args>
        static AllocatedOptional Emplace(Args &&... args) {
            return AllocatedOptional(std::make_unique<T>(std::forward<Args>(args)...));
        }

        static constexpr AllocatedOptional Empty() {
            return Result::PETAL_OPTIONAL_EMPTY;
        }

    public:
        Result GetResult() const { return m_result; }
        T *Value() { return m_present ? m_value.get() : nullptr; }
        bool HasValue() const { return m_present; }
        bool IsEmpty() const { return !HasValue(); }

        T *operator->() {
            assert(HasValue());
            return m_value.get();
        }

    private:
        std::unique_ptr<T> m_value;
        bool m_present;
        Result m_result;
    };

    template<typename T>
    class OptionalRef {
        static_assert(!IsResult<T>::value, "OptionalRef<Result> is forbidden because it creates constructor ambiguity.");

    public:
        OptionalRef(T &value)
            : m_value(&value),
              m_present(true),
              m_result(Result::SUCCESS) {
        }

        OptionalRef(Result result)
            : m_value(nullptr),
              m_present(false),
              m_result(result) {
            if (result == Result::SUCCESS) [[unlikely]] {
#ifndef NDEBUG
                __debugbreak();
#endif
            }
        }

        OptionalRef(const OptionalRef &other) = delete;

        OptionalRef(OptionalRef &&other) noexcept
            : m_value(other.m_value),
              m_present(other.m_present),
              m_result(other.m_result) {
        }

        OptionalRef &operator=(const OptionalRef &other) = delete;

        OptionalRef &operator=(OptionalRef &&other) noexcept {
            if (this == &other)
                return *this;

            m_value = other.m_value;
            m_present = other.m_present;
            m_result = other.m_result;

            other.m_value = nullptr;
            other.m_present = false;
            other.m_result = Result::PETAL_OPTIONAL_MOVED_OUT;

            return *this;
        }

        ~OptionalRef() {
            m_value = nullptr;
        }

        static constexpr OptionalRef Empty() {
            return Result::PETAL_OPTIONAL_EMPTY;
        }

    public:
        Result GetResult() const {
            return m_result;
        }

        T *Value() {
            return m_present ? m_value : nullptr;
        }

        bool HasValue() const {
            return m_present;
        }

        bool IsEmpty() const {
            return !HasValue();
        }

        T *operator->() {
            assert(HasValue());
            return m_value.get();
        }

    private:
        T *m_value;
        bool m_present;
        Result m_result;
    };

#define PETAL_CHECK_OPTIONAL(optional, loggerRef, message, ...) \
    do { \
        const auto& __optionalStored = (optional); \
        PETAL_CHECK_COND(__optionalStored.IsEmpty(), __optionalStored.GetResult(), loggerRef, message, ##__VA_ARGS__); \
    } while (0)

#define PETAL_CHECK_OPTIONAL_SILENT(optional) \
    do { \
        const auto& __optionalStored = optional; \
        PETAL_CHECK_COND_SILENT(__optionalStored.IsEmpty(), __optionalStored.GetResult()); \
    } while (0)
} // Petal
