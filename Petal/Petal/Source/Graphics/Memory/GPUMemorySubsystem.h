#pragma once

#include "Common.h"
#include "Buffers/BufferCreateInfo.h"

namespace Petal {
    class IBuffer;
    class CommandBuffer;
    class VulkanBuffer;
    class SwapchainBuffers;
    struct ImageLoaderSettings;
    struct TextureCreateInfo;
    class VulkanTexture;
    class GPUBuffer;
    class GraphicsContext;

    class GPUMemorySubsystem {
    public:
        GPUMemorySubsystem(
            GraphicsContext &renderer,
            const std::shared_ptr<Logger> &logger,
            Result &resultOut
        );

        ~GPUMemorySubsystem();

    public:
        // Create a texture.
        // A VulkanTexture is similar to a VulkanBuffer where it has full control over its memory, and it is not shared with other resources, unlike a GPUBuffer.
        AllocatedOptional<VulkanTexture> CreateTexture(
            const std::string &name,
            const TextureCreateInfo &textureCreateInfo
        );

        // Create a texture by loading from a file on disk
        // The size will always be the size of the loaded image, regardless of what was passed into TextureCreateInfo
        AllocatedOptional<VulkanTexture> LoadTextureFromDisk(
            const std::filesystem::path &path,
            const ImageLoaderSettings &loadingSettings,
            TextureCreateInfo textureCreateInfo
        );

        // Create a buffer
        // Independent means that the internal VulkanBuffer is exclusively for this GPUBuffer and is not shared
        AllocatedOptional<GPUBuffer> CreateIndependentBuffer(
            const std::string &name,
            glm::u32 size,
            const BufferCreateInfo &createInfo = {}
        );

        // Create a GPUBuffer which is backed by all or a subsection of a VulkanBuffer
        // size - size of the GPUBuffer
        // offset - where in the backingBuffer this buffer is located
        AllocatedOptional<GPUBuffer> CreateBackedBuffer(
            const std::string &name,
            glm::u32 size,
            const std::shared_ptr<VulkanBuffer> &backingBuffer
        );

        // Create a Vulkan buffer
        // You must create one (or more) GPUBuffer which use the VulkanBuffer as a backing buffer
        AllocatedOptional<VulkanBuffer> CreateVulkanBuffer(
            const std::string &name,
            glm::u32 size,
            const BufferCreateInfo &createInfo = {}
        );

        // Create a single VulkanBuffer with one GPUBuffer per swapchain image.
        // SwapchainBuffers contains a utility for writing to the correct buffer each frame, but Update() must be called every frame.
        AllocatedOptional<SwapchainBuffers> CreateSwapchainBuffers(
            const std::string &name,
            glm::u32 sizePerBuffer,
            const BufferCreateInfo &createInfo = {}
        );

        // Write to a buffer
        Result Write(
            const IBuffer &buffer,
            const void *data,
            glm::u32 size
        );

        // Write to a texture
        Result Write(
            const VulkanTexture &texture,
            const void *data,
            glm::u32 size
        );

    private:
        Result CreateTransferBuffer();

        Result CreateCommandBuffer();

    private:
        GraphicsContext &m_context;
        std::shared_ptr<Logger> m_logger;
        std::unique_ptr<VulkanBuffer> m_transferBuffer;
        std::unique_ptr<CommandBuffer> m_commandBuffer;
    };
} // Petal
