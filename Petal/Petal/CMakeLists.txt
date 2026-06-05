
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
        Source/Graphics/GraphicsSystem.cpp
        Source/Graphics/GraphicsSystem.h
        Source/Utils/AppInfo/EngineInfo.h
        Source/Utils/AppInfo/Version.h
        Source/Common.h
        Source/Utils/AppInfo/Version.cpp
        Source/Utils/Logging/Errors/ErrorHandler.h
        Source/Utils/Logging/Errors/Result.h
        Source/Utils/FormatString.h
        Source/Graphics/Internal/Vulkan.h
        Source/Graphics/Internal/RenderingDevice.cpp
        Source/Graphics/Internal/RenderingDevice.h
        Source/Utils/Logging/Errors/Optional.h
        Source/Utils/ConstraintMatching.cpp
        Source/Utils/ConstraintMatching.h
        Source/Utils/FormatContainers.h
        Source/Graphics/Internal/DeviceRequirements.cpp
        Source/Graphics/Internal/DeviceRequirements.h
        Source/Graphics/GraphicsContext.cpp
        Source/Graphics/GraphicsContext.h
        Source/Graphics/Internal/VulkanAllocator.cpp
        Source/Graphics/Internal/VulkanAllocator.h
        Source/Graphics/Internal/VulkanSwapchain.cpp
        Source/Graphics/Internal/VulkanSwapchain.h
        Source/Graphics/GraphicsSettings.h
        Source/Graphics/GraphicsSettings.cpp
        Source/Constants.h
        Source/Graphics/Internal/CommandBuffers/CommandBuffer.cpp
        Source/Graphics/Internal/CommandBuffers/CommandBuffer.h
        Source/Graphics/Internal/CommandBuffers/CommandBufferVector.cpp
        Source/Graphics/Internal/CommandBuffers/CommandBufferVector.h
        Source/Graphics/Internal/VulkanQueue.cpp
        Source/Graphics/Internal/VulkanQueue.h
        Source/Utils/CommonTraits.h
        Source/CopyMoveMacros.h
        Source/Graphics/Internal/Sync/VulkanFence.cpp
        Source/Graphics/Internal/Sync/VulkanFence.h
        Source/Graphics/Internal/Sync/VulkanSemaphore.cpp
        Source/Graphics/Internal/Sync/VulkanSemaphore.h
        Source/Utils/Color/Color.cpp
        Source/Utils/Color/Color.h
        Source/Graphics/Internal/CommandBuffers/CommandBufferRef.cpp
        Source/Graphics/Internal/CommandBuffers/CommandBufferRef.h
        Source/Graphics/Shaders/ShaderSubsystem.cpp
        Source/Graphics/Shaders/ShaderSubsystem.h
        Source/Graphics/Shaders/slang.h
        Source/Graphics/Shaders/ShaderType.h
        Source/Graphics/Shaders/ShaderType.cpp
        Source/Graphics/Shaders/ShaderAsset.h
        Source/Graphics/Shaders/IntermediateShaderResource.h
        Source/Utils/File/FileUtils.cpp
        Source/Utils/File/FileUtils.h
        Source/Utils/Timing/Timer.cpp
        Source/Utils/Timing/Timer.h
        Source/Graphics/Shaders/ShaderInfo.h
        Source/Graphics/Internal/VulkanShader.cpp
        Source/Graphics/Internal/VulkanShader.h
        Source/Graphics/Resources/ResourceType.h
        Source/Graphics/Resources/ShaderResource.h
        Source/Graphics/Resources/ResourceType.cpp
        Source/Graphics/Memory/Buffers/GPUBuffer.cpp
        Source/Graphics/Memory/Buffers/GPUBuffer.h
        Source/Graphics/Memory/GPUMemorySubsystem.cpp
        Source/Graphics/Memory/GPUMemorySubsystem.h
        Source/Graphics/Memory/Buffers/VulkanBuffer.cpp
        Source/Graphics/Memory/Buffers/VulkanBuffer.h
        Source/Graphics/Memory/Buffers/BufferType.cpp
        Source/Graphics/Memory/Buffers/BufferType.h
        Source/Utils/Hash.h
        Source/Graphics/Resources/ShaderResource.cpp
        Source/Graphics/Internal/VulkanGraphicsPipeline.cpp
        Source/Graphics/Internal/VulkanGraphicsPipeline.h
        Assets/Shaders/Common.h
        Source/Utils/Scheduler.cpp
        Source/Utils/Scheduler.h
        Source/Resources/MeshBuilder.cpp
        Source/Resources/MeshBuilder.h
        Source/Graphics/Other/IndexType.cpp
        Source/Graphics/Other/IndexType.h
        Source/Rendering/Resources/MeshResource.cpp
        Source/Rendering/Resources/MeshResource.h
        Source/Rendering/Renderer.cpp
        Source/Rendering/Renderer.h
        Source/Graphics/Memory/AllocationTracker.cpp
        Source/Graphics/Memory/AllocationTracker.h
        Source/Rendering/RendererSettings.cpp
        Source/Rendering/RendererSettings.h
        Source/Graphics/Memory/Buffers/IBuffer.h
        Source/Resources/ImageLoaderSettings.cpp
        Source/Resources/ImageLoaderSettings.h
        Source/Utils/CheckSettingsMacro.h
        Source/Resources/Image.cpp
        Source/Resources/Image.h
        Source/Graphics/Memory/Textures/VulkanTexture.cpp
        Source/Graphics/Memory/Textures/VulkanTexture.h
        Source/Graphics/Memory/Textures/TextureCreateInfo.cpp
        Source/Graphics/Memory/Textures/TextureCreateInfo.h
        Source/Resources/Model.cpp
        Source/Resources/Model.h
        Source/Rendering/Resources/ModelResource.cpp
        Source/Rendering/Resources/ModelResource.h
        Source/Graphics/VertexType.h
        Source/Rendering/Camera/Camera.cpp
        Source/Rendering/Camera/Camera.h
        Source/Window/MouseButton.h
        Source/Window/Key.h
        Assets/Shaders/Petal/Math.h
        Source/Utils/StableVector.h
        Source/Graphics/Internal/RenderTarget.cpp
        Source/Graphics/Internal/RenderTarget.h
        Source/Graphics/Internal/ITexture.h
        Source/Rendering/FrameGraph/RenderPass.cpp
        Source/Rendering/FrameGraph/RenderPass.h
        Source/Utils/INameable.h
        Source/Graphics/Memory/IVulkanResource.h
        Source/Rendering/FrameGraph/FrameGraph.cpp
        Source/Rendering/FrameGraph/FrameGraph.h
        Source/Rendering/FrameGraph/PresentRenderPass.cpp
        Source/Rendering/FrameGraph/PresentRenderPass.h
        Source/Rendering/FrameGraph/ResourceUsage.cpp
        Source/Rendering/FrameGraph/ResourceUsage.h
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
target_precompile_headers(Petal PRIVATE Source/Graphics/Shaders/slang.h)

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

# stb-image
target_link_libraries(Petal PRIVATE stb)

# assimp
find_package(assimp CONFIG REQUIRED)
target_link_libraries(Petal PRIVATE assimp::assimp)
