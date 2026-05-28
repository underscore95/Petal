#include "GPUMemorySubsystem.h"

#include "Buffers/GPUBuffer.h"
#include "Graphics/Internal/VulkanQueue.h"
#include "Resources/Image.h"
#include "Textures/VulkanTexture.h"
#include "Timing/Timer.h"

namespace Petal {
    GPUMemorySubsystem::GPUMemorySubsystem(
        GraphicsContext &renderer,
        const std::shared_ptr<Logger> &logger,
        Result &resultOut
    )
        : m_context(renderer),
          m_logger(logger) {
        logger->SetMuted(true); // todo configurable

        resultOut = CreateTransferBuffer();
        if (resultOut != Result::SUCCESS) return;

        resultOut = CreateCommandBuffer();
        if (resultOut != Result::SUCCESS) return;

        m_logger->Verbose("Initialized GPUBufferSubsystem");
    }

    GPUMemorySubsystem::~GPUMemorySubsystem() = default;

    AllocatedOptional<VulkanTexture> GPUMemorySubsystem::CreateTexture(
        const std::string &name,
        const TextureCreateInfo &textureCreateInfo
    ) {
        Result resultOut;
        auto texture = std::make_unique<VulkanTexture>(m_context, m_logger, name, textureCreateInfo, resultOut);
        if (resultOut != Result::SUCCESS) return resultOut;
        return texture;
    }

    AllocatedOptional<VulkanTexture> GPUMemorySubsystem::LoadTextureFromDisk(
        const std::filesystem::path &path,
        const ImageLoaderSettings &loadingSettings,
        TextureCreateInfo textureCreateInfo
    ) {
        Result resultOut;
        Image image(m_logger, path, loadingSettings, resultOut);
        PETAL_CHECK_COND_SILENT(resultOut != Result::SUCCESS, resultOut);
        textureCreateInfo.Size = glm::uvec3{image.GetDimensions().x, image.GetDimensions().y, 1};

        AllocatedOptional<VulkanTexture> texture = CreateTexture(path.string(), textureCreateInfo);
        PETAL_CHECK_OPTIONAL_SILENT(texture);

        assert(image.GetSize() == texture->GetSize());
        resultOut = Write(*texture.Value(), image.GetData(), image.GetSize());
        PETAL_CHECK_COND_SILENT(resultOut != Result::SUCCESS, resultOut);

        return texture;
    }

    AllocatedOptional<GPUBuffer> GPUMemorySubsystem::CreateIndependentBuffer(
        const std::string &name,
        glm::u32 size,
        const BufferCreateInfo &createInfo
    ) {
        AllocatedOptional<VulkanBuffer> bufferOpt = CreateVulkanBuffer(name, size, createInfo);
        PETAL_CHECK_OPTIONAL_SILENT(bufferOpt);

        Optional<Allocation> allocationOpt = bufferOpt->GetAllocations().Allocate(size);
        PETAL_CHECK_OPTIONAL(allocationOpt, m_logger, "Failed to create independent buffer (this should never happen...)");

        auto gpuBuffer = std::make_unique<GPUBuffer>(m_context, name, bufferOpt.Release(), *allocationOpt.Value());
        return gpuBuffer;
    }

    AllocatedOptional<GPUBuffer> GPUMemorySubsystem::CreateBackedBuffer(
        const std::string &name,
        glm::u32 size,
        const std::shared_ptr<VulkanBuffer> &backingBuffer
    ) {
        PETAL_CHECK_COND(backingBuffer == nullptr, Result::PETAL_UNEXPECTED_NULLPTR, m_logger, "Backing buffer was nullptr");
        PETAL_CHECK_COND(size == 0, Result::PETAL_BUFFER_CREATION_FAILED, m_logger, "GPUBuffer size was 0");

        Optional<Allocation> allocationOpt = backingBuffer->GetAllocations().Allocate(size);
        PETAL_CHECK_OPTIONAL(allocationOpt, m_logger, "Failed to create backed buffer of size {}", size);
        assert(allocationOpt->Size == size);

        m_logger->Info("Created backed buffer of size {} at location {} (name {})", size, allocationOpt->Location, name);
        return std::make_unique<GPUBuffer>(m_context, name, backingBuffer, *allocationOpt.Value());
    }

    Result GPUMemorySubsystem::Write(
        const IBuffer &buffer,
        const void *data,
        glm::u32 size
    ) {
        Timer timer;

        PETAL_CHECK_COND(data == nullptr, Result::VMA_BUFFER_WRITE_FAILED, m_logger, "Attempted to write to buffer {} but data was nullptr", buffer.GetName());
        PETAL_CHECK_COND(size == 0, Result::VMA_BUFFER_WRITE_FAILED, m_logger, "Attempted to write to buffer {} but size was 0", buffer.GetName());
        PETAL_CHECK_COND(
            size > buffer.GetSize(),
            Result::VMA_BUFFER_WRITE_FAILED,
            m_logger,
            "Attempted to write to buffer {} with size {} but amount of data to copy was {}", buffer.GetName(), buffer.GetSize(), size
        );

        void *dest = nullptr;
        VkResult res = vmaMapMemory(m_context.GetAllocator()->GetHandle(), m_transferBuffer->GetVMAAllocation(), &dest);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VMA_BUFFER_WRITE_FAILED, m_logger, "Failed to map transfer buffer: {}", res);

        // Write in blocks
        const glm::u32 blockSize = m_transferBuffer->GetSize();
        glm::u32 bytesRemaining = size;
        const glm::u32 bufferOffset = buffer.GetOffset();
        m_logger->Verbose("Writing to buffer at location {}, {} bytes remaining", bufferOffset, bytesRemaining);
        for (glm::u32 blockIndex = 0; blockIndex <= size / blockSize; blockIndex++) {
            glm::u32 blockOffset = blockIndex * blockSize;
            glm::u32 bytesToWrite = glm::min(blockSize, bytesRemaining);
            memcpy(static_cast<char *>(dest) + blockOffset, static_cast<const char *>(data) + blockOffset, bytesToWrite);

            Result result = m_commandBuffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

            VkBufferCopy copy = {
                .srcOffset = 0,
                .dstOffset = blockOffset + bufferOffset,
                .size = bytesToWrite
            };
            vkCmdCopyBuffer(
                m_commandBuffer->GetHandle(),
                m_transferBuffer->GetHandle(),
                buffer.GetHandle(),
                1,
                &copy
            );

            result = m_commandBuffer->End();
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

            // todo better sync?
            result = m_context.GetSwapchain().SubmitBlockingCommand(m_commandBuffer->GetHandle());
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

            bytesRemaining -= bytesToWrite;
            m_logger->Verbose("Writing to buffer, {} bytes remaining. Block index: {}, block offset: {}, block size: {}", bytesRemaining, blockIndex, blockOffset, bytesToWrite);
            if (bytesRemaining == 0) break;
        }

        assert(bytesRemaining == 0);

        vmaUnmapMemory(m_context.GetAllocator()->GetHandle(), m_transferBuffer->GetVMAAllocation());

        m_logger->Verbose("Finished writing to buffer in {} ms", timer.MillisSinceStart());
        return Result::SUCCESS;
    }

    Result GPUMemorySubsystem::Write(
        const VulkanTexture &texture,
        const void *data,
        glm::u32 size
    ) {
        Timer timer;

        PETAL_CHECK_COND(data == nullptr, Result::VMA_TEXTURE_WRITE_FAILED, m_logger, "Attempted to write to texture {} but data was nullptr", texture.GetName());
        PETAL_CHECK_COND(size == 0, Result::VMA_TEXTURE_WRITE_FAILED, m_logger, "Attempted to write to texture {} but size was 0", texture.GetName());
        PETAL_CHECK_COND(
            size > texture.GetSize(),
            Result::VMA_TEXTURE_WRITE_FAILED,
            m_logger,
            "Attempted to write to texture {} with size {} but amount of data to copy was {}", texture.GetName(), texture.GetSize(), size
        );

        // todo support writing in blocks like writing to buffers
        PETAL_CHECK_COND(
            texture.GetSize() > m_transferBuffer->GetSize(),
            Result::VMA_TEXTURE_WRITE_FAILED,
            m_logger,
            "Attempted to write to texture {} with size {} but it was bigger than the transfer buffer which is {}",
            texture.GetName(), texture.GetSize(), m_transferBuffer->GetSize()
        );

        void *dest = nullptr;
        VkResult res = vmaMapMemory(m_context.GetAllocator()->GetHandle(), m_transferBuffer->GetVMAAllocation(), &dest);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VMA_BUFFER_WRITE_FAILED, m_logger, "Failed to map transfer buffer: {}", res);
        memcpy(dest, data, size);

        Result result = m_commandBuffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        VkImageMemoryBarrier2 imageBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .srcQueueFamilyIndex = m_context.GetDevice()->GetGraphicsQueueFamily().GetQueueFamilyIndex(),
            .dstQueueFamilyIndex = m_context.GetDevice()->GetGraphicsQueueFamily().GetQueueFamilyIndex(),
            .image = texture.GetHandle(),
            .subresourceRange = GraphicsContext::DEFAULT_IMAGE_SUBRESOURCE_RANGE
        };

        m_context.CmdTransitionImage(
            m_commandBuffer->GetHandle(),
            texture.GetHandle(),
            imageBarrier.oldLayout,
            imageBarrier.newLayout,
            imageBarrier
        );

        VkBufferImageCopy copy = {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                .aspectMask = texture.GetAspectMask(),
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .imageOffset = {0, 0, 0},
            .imageExtent = {.width = texture.GetDimensions().x, .height = texture.GetDimensions().y, .depth = texture.GetDimensions().z}
        };

        vkCmdCopyBufferToImage(
            m_commandBuffer->GetHandle(),
            m_transferBuffer->GetHandle(),
            texture.GetHandle(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &copy
        );

        imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageBarrier.dstAccessMask = VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT |
                                     VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        m_context.CmdTransitionImage(
            m_commandBuffer->GetHandle(),
            texture.GetHandle(),
            imageBarrier.oldLayout,
            imageBarrier.newLayout,
            imageBarrier
        );

        result = m_commandBuffer->End();
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        // todo better sync?
        result = m_context.GetSwapchain().SubmitBlockingCommand(m_commandBuffer->GetHandle());
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        vmaUnmapMemory(m_context.GetAllocator()->GetHandle(), m_transferBuffer->GetVMAAllocation());

        m_logger->Verbose("Finished writing to buffer in {} ms", timer.MillisSinceStart());
        return Result::SUCCESS;
    }

    Result GPUMemorySubsystem::CreateTransferBuffer() {
        constexpr glm::u32 TRANSFER_BUFFER_SIZE = 256 * 1024 * 1024; // 256mb
        // todo: multiple transfer buffers
        // for smaller uploads, we can use the small buffer which is much faster than the big buffer

        BufferCreateInfo createInfo = {
            .BufferType = BufferType::STORAGE_BUFFER,
            .DeviceLocal = true,
            .HostVisible = true,
            .IsTransferDest = true,
            .IsTransferSource = true
        };

        AllocatedOptional<VulkanBuffer> buffer = CreateVulkanBuffer("Transfer Buffer", TRANSFER_BUFFER_SIZE, createInfo);
        PETAL_CHECK_OPTIONAL_SILENT(buffer);

        m_transferBuffer = buffer.Release();
        return Result::SUCCESS;
    }

    Result GPUMemorySubsystem::CreateCommandBuffer() {
        AllocatedOptional<CommandBuffer> commandBufferOptional = m_context.CreateCommandBuffer(
            m_context.GetDevice()->GetGraphicsQueueFamily(), // todo transfer queue
            VK_COMMAND_BUFFER_LEVEL_PRIMARY
        );

        PETAL_CHECK_OPTIONAL(commandBufferOptional, m_logger, "Failed to create transfer command buffer for GPUBufferSubsystem");
        m_commandBuffer = commandBufferOptional.Release();

        return Result::SUCCESS;
    }

    AllocatedOptional<VulkanBuffer> GPUMemorySubsystem::CreateVulkanBuffer(
        const std::string &name,
        glm::u32 size,
        const BufferCreateInfo &createInfo
    ) {
        Result result;
        auto buffer = std::make_unique<VulkanBuffer>(
            m_context,
            m_logger,
            name,
            size,
            createInfo,
            result
        );

        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        return buffer;
    }
} // Petal
