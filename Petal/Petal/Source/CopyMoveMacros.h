#pragma once

#define DISABLE_COPY(T)            \
    T(const T&) = delete;         \
    T& operator=(const T&) = delete;

#define DISABLE_MOVE(T)            \
    T(T&&) = delete;              \
    T& operator=(T&&) = delete;

#define DISABLE_COPY_AND_MOVE(T)   \
    DISABLE_COPY(T)               \
    DISABLE_MOVE(T)