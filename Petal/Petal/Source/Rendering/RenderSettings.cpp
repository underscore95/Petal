#include "RenderSettings.h"

#define CHECK_RENDER_SETTINGS(cond, fmt, ...) \
    PETAL_CHECK_COND( \
        cond, \
        Result::PETAL_INVALID_RENDER_SETTINGS, \
        logger, \
        "Invalid RenderSettings! {}", \
        std::format(fmt, ##__VA_ARGS__)                  \
    )

Petal::Result Petal::RenderSettings::IsValid(Ref<Logger> logger) const {
    CHECK_RENDER_SETTINGS(NumSwapchainImages < 2, "Not enough swapchain images: {}", NumSwapchainImages);
    CHECK_RENDER_SETTINGS(NumSwapchainImages > 3, "Too many swapchain images: {}", NumSwapchainImages);

    CHECK_RENDER_SETTINGS(PreferredPresentMode.empty(), "PreferredPresentMode is empty");
    CHECK_RENDER_SETTINGS(
        PreferredPresentMode.size() != std::unordered_set(PreferredPresentMode.begin(), PreferredPresentMode.end()).size(),
        "PreferredPresentMode contains duplicate elements"
    );

#undef CHECK_RENDER_SETTINGS

    return Result::SUCCESS;
}
