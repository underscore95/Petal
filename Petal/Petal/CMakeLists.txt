
add_library(Petal SHARED
        Source/Engine.cpp
        Source/Engine.h
        Source/pch.h
        Source/Utils/Logging/Logger.cpp
        Source/Utils/Logging/Logger.h
        Source/Utils/Logging/LogLevel.h
        Source/Utils/Logging/LoggerSystem.cpp
        Source/Utils/Logging/LoggerSystem.h
        Source/Utils/Memory/MemorySystem.cpp
        Source/Utils/Memory/MemorySystem.h
        Source/Window/Window.cpp
        Source/Window/Window.h
        Source/Window/WindowSystem.cpp
        Source/Window/WindowSystem.h
        Source/Rendering/RenderingSystem.cpp
        Source/Rendering/RenderingSystem.h
        Source/Utils/AppInfo/EngineInfo.h
        Source/Utils/AppInfo/Version.h
        Source/Common.h
        Source/Utils/AppInfo/Version.cpp
        Source/Utils/Logging/Errors/ErrorHandler.h
        Source/Utils/Logging/Errors/Result.h
        Source/Utils/FormatString.h
        Source/Rendering/Internal/VkFormat.h
        Source/Rendering/Internal/RenderingDevice.cpp
        Source/Rendering/Internal/RenderingDevice.h
        Source/Utils/Logging/Errors/Optional.h
        Source/Utils/ConstraintMatching.cpp
        Source/Utils/ConstraintMatching.h
        Source/Utils/FormatContainers.h
        Source/Rendering/Internal/DeviceRequirements.cpp
        Source/Rendering/Internal/DeviceRequirements.h
        Source/Rendering/Renderer.cpp
        Source/Rendering/Renderer.h
        Source/Rendering/Internal/VulkanAllocator.cpp
        Source/Rendering/Internal/VulkanAllocator.h
        Source/Rendering/Internal/VulkanSwapchain.cpp
        Source/Rendering/Internal/VulkanSwapchain.h
        Source/Rendering/RenderSettings.h
        Source/Rendering/RenderSettings.cpp
        Source/Constants.h
        Source/Rendering/Internal/CommandBuffers/CommandBuffer.cpp
        Source/Rendering/Internal/CommandBuffers/CommandBuffer.h
        Source/Rendering/Internal/CommandBuffers/CommandBufferVector.cpp
        Source/Rendering/Internal/CommandBuffers/CommandBufferVector.h
        Source/Rendering/Internal/VulkanQueue.cpp
        Source/Rendering/Internal/VulkanQueue.h
        Source/Utils/CommonTraits.h
        Source/CopyMoveMacros.h
        Source/Rendering/Internal/Sync/VulkanFence.cpp
        Source/Rendering/Internal/Sync/VulkanFence.h
        Source/Rendering/Internal/Sync/VulkanSemaphore.cpp
        Source/Rendering/Internal/Sync/VulkanSemaphore.h
        Source/Utils/Color/Color.cpp
        Source/Utils/Color/Color.h
        Source/Rendering/Internal/CommandBuffers/CommandBufferRef.cpp
        Source/Rendering/Internal/CommandBuffers/CommandBufferRef.h
        Source/Rendering/Shaders/ShaderSubsystem.cpp
        Source/Rendering/Shaders/ShaderSubsystem.h
        Source/Rendering/Shaders/slang.h
        Source/Rendering/Shaders/ShaderType.h
        Source/Rendering/Shaders/ShaderType.cpp
        Source/Rendering/Shaders/ShaderAsset.h
        Source/Rendering/Shaders/VulkanShaderResource.cpp
        Source/Rendering/Shaders/VulkanShaderResource.h
        Source/Rendering/Shaders/IntermediateShaderResource.h
        Source/Utils/File/FileUtils.cpp
        Source/Utils/File/FileUtils.h
        Source/Utils/Timing/Timer.cpp
        Source/Utils/Timing/Timer.h
        Source/Rendering/Shaders/ShaderInfo.h
        Source/Rendering/Internal/VulkanShader.cpp
        Source/Rendering/Internal/VulkanShader.h
        Source/Rendering/Resources/ResourceType.h
        Source/Rendering/Resources/ShaderResource.h
        Source/Rendering/Resources/ResourceType.cpp
        Source/Rendering/Memory/GPUBuffer.cpp
        Source/Rendering/Memory/GPUBuffer.h
        Source/Rendering/Memory/GPUBufferSubsystem.cpp
        Source/Rendering/Memory/GPUBufferSubsystem.h
        Source/Rendering/Memory/VulkanBuffer.cpp
        Source/Rendering/Memory/VulkanBuffer.h
        Source/Rendering/Memory/BufferType.cpp
        Source/Rendering/Memory/BufferType.h
        Source/Utils/Hash.h
        Source/Rendering/Resources/ShaderResource.cpp
        Source/Rendering/Internal/VulkanGraphicsPipeline.cpp
        Source/Rendering/Internal/VulkanGraphicsPipeline.h
)

# Macros
target_compile_definitions(Petal PUBLIC
        PETAL_VERSION_MAJOR=0
        PETAL_VERSION_MINOR=1
        PETAL_VERSION_PATCH=0
        PETAL_VERSION_VARIANT=0
)
target_compile_definitions(Petal PUBLIC PETAL_EXPORTS=1)

# Include directories
target_include_directories(Petal PUBLIC Source/)
target_include_directories(Petal PUBLIC Source/Utils)

# PCHs
target_precompile_headers(Petal PRIVATE Source/pch.h)
target_precompile_headers(Petal PRIVATE Source/Common.h)
target_precompile_headers(Petal PRIVATE Source/Rendering/Shaders/slang.h)

# Other options
if (MSVC)
    target_compile_options(Petal PRIVATE /Zc:preprocessor)
endif ()

# Libraries
# Vulkan
find_package(Vulkan)
target_link_libraries(Petal PRIVATE Vulkan::Vulkan)
target_compile_definitions(Petal PUBLIC PETAL_VULKAN=1)

# Vulkan Utility
find_package(VulkanUtilityLibraries CONFIG REQUIRED)
target_link_libraries(Petal PRIVATE Vulkan::UtilityHeaders)

# Vulkan Memory Allocator
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
target_link_libraries(Petal PUBLIC GPUOpen::VulkanMemoryAllocator)

# GLFW
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(Petal PRIVATE glfw)

# GLM
find_package(glm CONFIG REQUIRED)
target_link_libraries(Petal PUBLIC glm::glm)

# slang
find_package(slang CONFIG REQUIRED)
target_link_libraries(Petal PRIVATE slang::slang)

# magic-enum
find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(Petal PRIVATE magic_enum::magic_enum)