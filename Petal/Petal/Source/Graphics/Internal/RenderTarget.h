#pragma once

namespace Petal {
    class ITexture;

    struct RenderTarget {
        std::shared_ptr<ITexture> Color;
        std::shared_ptr<ITexture> Depth;

        Optional<VkRenderingAttachmentInfo> CreateColorAttachment() const;

        Optional<VkRenderingAttachmentInfo> CreateDepthAttachment() const;
    };
} // Petal
