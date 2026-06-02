#pragma once

namespace Petal {
    class IHasVulkanImage;

    struct RenderTarget {
        std::shared_ptr<IHasVulkanImage> Color;
        std::shared_ptr<IHasVulkanImage> Depth;

        Optional<VkRenderingAttachmentInfo> CreateColorAttachment() const;

        Optional<VkRenderingAttachmentInfo> CreateDepthAttachment() const;
    };
} // Petal
