#include "GPUBufferSubsystem.h"

#include "GPUBuffer.h"
#include "Timing/Timer.h"

namespace Petal {
    GPUBufferSubsystem::GPUBufferSubsystem(
        GraphicsContext &renderer,
        const std::shared_ptr<Logger> &logger,
        Result &resultOut
    )
        : m_renderer(renderer),
          m_logger(logger) {
        resultOut = CreateTransferBuffer();
        if (resultOut != Result::SUCCESS) return;

        resultOut = CreateCommandBuffer();
        if (resultOut != Result::SUCCESS) return;

        m_logger->Verbose("Initialized GPUBufferSubsystem");
    }

    GPUBufferSubsystem::~GPUBufferSubsystem() = default;

    AllocatedOptional<GPUBuffer> GPUBufferSubsystem::CreateIndependentBuffer(
        const std::string &name,
        glm::u32 size,
        const BufferCreateInfo &createInfo
    ) {
        AllocatedOptional<VulkanBuffer> bufferOpt = CreateVulkanBuffer(name, size, createInfo);
        PETAL_CHECK_OPTIONAL_SILENT(bufferOpt);

        Optional<Allocation> allocationOpt = bufferOpt->GetAllocations().Allocate(size);
        PETAL_CHECK_OPTIONAL(allocationOpt, m_logger, "Failed to create independent buffer (this should never happen...)");

        auto gpuBuffer = std::make_unique<GPUBuffer>(name, bufferOpt.Release(), *allocationOpt.Value());
        return gpuBuffer;
    }

    AllocatedOptional<GPUBuffer> GPUBufferSubsystem::CreateBackedBuffer(
        const std::string &name,
        glm::u32 size,
        std::shared_ptr<VulkanBuffer> backingBuffer
    ) {
        PETAL_CHECK_COND(backingBuffer == nullptr, Result::PETAL_UNEXPECTED_NULLPTR, m_logger, "Backing buffer was nullptr");
        PETAL_CHECK_COND(size == 0, Result::PETAL_BUFFER_CREATION_FAILED, m_logger, "GPUBuffer size was 0");

        Optional<Allocation> allocationOpt = backingBuffer->GetAllocations().Allocate(size);
        PETAL_CHECK_OPTIONAL(allocationOpt, m_logger, "Failed to create backed buffer of size {}", size);

        return std::make_unique<GPUBuffer>(name, backingBuffer, *allocationOpt.Value());
    }

    Result GPUBufferSubsystem::Write(
        const GPUBuffer &buffer,
        const void *data,
        glm::u32 size,
        glm::u32 bufferOffset
    ) {
        Timer timer;

        PETAL_CHECK_COND(data == nullptr, Result::VMA_BUFFER_WRITE_FAILED, m_logger, "Attempted to write to buffer {} but data was nullptr", buffer.GetName());
        PETAL_CHECK_COND(size == 0, Result::VMA_BUFFER_WRITE_FAILED, m_logger, "Attempted to write to buffer {} but size was 0", buffer.GetName());
        PETAL_CHECK_COND(
            bufferOffset + size > buffer.GetAllocation().Size,
            Result::VMA_BUFFER_WRITE_FAILED,
            m_logger,
            "Attempted to write to buffer {} with size {} but offset ({}) + size ({}) was {}", buffer.GetName(), buffer.GetAllocation().Size, bufferOffset, size, bufferOffset + size
        );


        void *ptr = nullptr;
        VkResult res = vmaMapMemory(m_renderer.GetAllocator()->GetAllocator(), m_transferBuffer->GetVMAAllocation(), &ptr);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VMA_BUFFER_WRITE_FAILED, m_logger, "Failed to map transfer buffer: {}", res);

        // Write in blocks
        glm::u32 blockSize = m_transferBuffer->GetSize();
        glm::u32 bytesRemaining = size;
        m_logger->Verbose("Writing to buffer, {} bytes remaining", bytesRemaining);
        for (glm::u32 blockIndex = 0; blockIndex <= size / blockSize; blockIndex++) {
            glm::u32 blockOffset = blockIndex * blockSize;
            glm::u32 bytesToWrite = glm::min(blockSize, bytesRemaining);
            // ReSharper disable once CppDFANullDereference
            memcpy(ptr, static_cast<const char *>(data) + blockOffset, bytesToWrite);

            Result result = m_commandBuffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

            VkBufferCopy copy = {
                .srcOffset = 0,
                .dstOffset = blockOffset,
                .size = bytesToWrite
            };
            vkCmdCopyBuffer(
                m_commandBuffer->GetHandle(),
                m_transferBuffer->GetHandle(),
                buffer.GetBackingBuffer()->GetHandle(),
                1,
                &copy
            );

            result = m_commandBuffer->End();
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

            // todo better sync?
            result = m_renderer.GetSwapchain().SubmitBlockingCommand(m_commandBuffer->GetHandle());
            PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

            bytesRemaining -= bytesToWrite;
            m_logger->Verbose("Writing to buffer, {} bytes remaining. Block index: {}, block offset: {}, block size: {}", bytesRemaining, blockIndex, blockOffset, bytesToWrite);
            if (bytesRemaining == 0) break;
        }

        assert(bytesRemaining == 0);

        vmaUnmapMemory(m_renderer.GetAllocator()->GetAllocator(), m_transferBuffer->GetVMAAllocation());

        m_logger->Verbose("Finished writing to buffer in {} ms", timer.MillisSinceStart());
        return Result::SUCCESS;
    }

    Result GPUBufferSubsystem::CreateTransferBuffer() {
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

    Result GPUBufferSubsystem::CreateCommandBuffer() {
        AllocatedOptional<CommandBuffer> commandBufferOptional = m_renderer.CreateCommandBuffer(
            m_renderer.GetDevice()->GetGraphicsQueueFamily(), // todo transfer queue
            VK_COMMAND_BUFFER_LEVEL_PRIMARY
        );

        PETAL_CHECK_OPTIONAL(commandBufferOptional, m_logger, "Failed to create transfer command buffer for GPUBufferSubsystem");
        m_commandBuffer = commandBufferOptional.Release();

        return Result::SUCCESS;
    }

    AllocatedOptional<VulkanBuffer> GPUBufferSubsystem::CreateVulkanBuffer(
        const std::string &name,
        glm::u32 size,
        const BufferCreateInfo &createInfo
    ) {
        Result result;
        auto buffer = std::make_unique<VulkanBuffer>(
            m_renderer,
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
