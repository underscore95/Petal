#pragma once

namespace Petal {
    class ITexture;

    struct RenderTarget {
        struct ColorAttachment {
            std::shared_ptr<ITexture> Texture;
            glm::vec4 ClearColor;
        };

        std::vector<ColorAttachment> Colors;
        std::shared_ptr<ITexture> Depth;

        std::vector<VkRenderingAttachmentInfo> CreateColorAttachments() const;

        Optional<VkRenderingAttachmentInfo> CreateDepthAttachment() const;
    };
} // Petal
