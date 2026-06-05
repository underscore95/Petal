#pragma once

#include "Common.h"

namespace Petal {
    struct ResourceUsage {
        VkPipelineStageFlags2 StageMask;
        VkAccessFlags2 AccessMask;
        Optional<VkImageLayout> ImageLayout;

        ResourceUsage(
            VkPipelineStageFlags2 stageMask,
            VkAccessFlags2 accessMask,
            Optional<VkImageLayout> imageLayout = Result::PETAL_OPTIONAL_EMPTY
        );

        static ResourceUsage Undefined();

        static ResourceUsage ColorAttachment();

        static ResourceUsage DepthAttachmentRead();

        static ResourceUsage DepthAttachmentWrite();

        static ResourceUsage DepthAttachmentReadWrite();

        static ResourceUsage Sampled(
            VkPipelineStageFlags2 shaderStages
        );

        static ResourceUsage StorageRead(
            VkPipelineStageFlags2 shaderStages
        );

        static ResourceUsage StorageWrite(
            VkPipelineStageFlags2 shaderStages
        );

        static ResourceUsage StorageReadWrite(
            VkPipelineStageFlags2 shaderStages
        );

        static ResourceUsage UniformBuffer(
            VkPipelineStageFlags2 shaderStages
        );

        static ResourceUsage VertexBuffer();

        static ResourceUsage IndexBuffer();

        static ResourceUsage IndirectBuffer();

        static ResourceUsage TransferSource();

        static ResourceUsage TransferDestination();

        static ResourceUsage Present();
    };
}
