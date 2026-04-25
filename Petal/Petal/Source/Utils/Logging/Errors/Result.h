#pragma once

namespace Petal {
    enum class Result {
        SUCCESS,

        // Vulkan Errors
        VULKAN_INSTANCE_CREATION_FAILED,
        VULKAN_CANNOT_FIND_ADDRESS_VK_CREATE_DEBUG_UTILS_MESSENGER,
        VULKAN_DEBUG_UTILS_MESSENGER_CREATION_FAILED,
        VULKAN_SURFACE_CREATION_FAILED,
        VULKAN_DEVICE_CREATION_FAILED,
        VULKAN_VMA_CREATION_FAILED,
        VULKAN_SWAPCHAIN_CREATION_FAILED,
        VULKAN_COMMAND_POOL_CREATION_FAILED,
        VULKAN_COMMAND_BUFFER_CREATION_FAILED,
        VULKAN_COMMAND_BUFFER_BEGIN_FAILED,
        VULKAN_COMMAND_BUFFER_END_FAILED,
        VULKAN_FENCE_CREATION_FAILED,
        VULKAN_SEMAPHORE_CREATION_FAILED,
        VULKAN_DEVICE_WAIT_IDLE_FAILED,
        VULKAN_PIPELINE_CREATION_FAILED,

        // Slang errors
        SLANG_INIT_FAILED,
        SLANG_SHADER_COMPILATION_FAILED,

        // Render Errors
        PETAL_INVALID_RENDER_SETTINGS,
        PETAL_INVALID_QUEUE_FAMILY,
        PETAL_BEGIN_RENDER_FAILED,
        PETAL_END_RENDER_FAILED,
        PETAL_FRAME_COMMAND_SUBMIT_FAILED,
        PETAL_PRESENT_FRAME_FAILED,
        PETAL_WINDOW_RESIZED, // Window was resized, the swapchain needs to be recreated

        // Other Errors
        PETAL_OPTIONAL_MOVED_OUT, // Contents of this optional were moved into another optional
        PETAL_OPTIONAL_RELEASED, // Contents of this optional were moved into another variable
        PETAL_OPTIONAL_EMPTY, // Created with no value, may not be an error
    };

    template<typename T>
    struct IsResult : std::is_same<T, Result> {
    };
} // Petal
