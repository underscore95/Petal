#pragma once

// DO NOT INCLUDE THIS INTO ANY HEADERS!

// A macro which is useful for checking that a settings object is valid
// for example, see GraphicsSettings.cpp

// EXAMPLE:
// #define SETTINGS_NAME "GraphicsSettings"
// #include "CheckSettingsMacro.h"
// CHECK_SETTINGS(NumSwapchainImages < 2, "Not enough swapchain images: {}", NumSwapchainImages);

#ifndef SETTINGS_NAME
// ReSharper disable once CppStaticAssertFailure
static_assert(false, "SETTINGS_NAME must be defined before using CHECK_SETTINGS");
#endif

#define CHECK_SETTINGS(cond, fmt, ...) \
    PETAL_CHECK_COND( \
        cond, \
        Result::PETAL_INVALID_SETTINGS, \
        logger, \
        "Invalid {}! {}", \
        SETTINGS_NAME, \
        std::format(fmt, ##__VA_ARGS__) \
    )