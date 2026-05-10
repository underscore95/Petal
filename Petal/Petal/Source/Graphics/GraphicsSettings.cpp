#include "GraphicsSettings.h"

#define CHECK_SETTINGS(cond, fmt, ...) \
    PETAL_CHECK_COND( \
        cond, \
        Result::PETAL_INVALID_SETTINGS, \
        logger, \
        "Invalid GraphicsSettings! {}", \
        std::format(fmt, ##__VA_ARGS__)                  \
    )

Petal::Result Petal::GraphicsSettings::IsValid(std::shared_ptr<Logger> logger) const {
    CHECK_SETTINGS(NumSwapchainImages < 2, "Not enough swapchain images: {}", NumSwapchainImages);
    CHECK_SETTINGS(NumSwapchainImages > 3, "Too many swapchain images: {}", NumSwapchainImages);

    CHECK_SETTINGS(PreferredPresentMode.empty(), "PreferredPresentMode is empty");
    CHECK_SETTINGS(
        PreferredPresentMode.size() != std::unordered_set(PreferredPresentMode.begin(), PreferredPresentMode.end()).size(),
        "PreferredPresentMode contains duplicate elements"
    );

    if (PushConstantSize > 128) {
        logger->Warn("GraphicsSettings push constant size was {}, some GPUs may not support more than 128 bytes.", PushConstantSize);
    }

#undef CHECK_SETTINGS

    return Result::SUCCESS;
}
