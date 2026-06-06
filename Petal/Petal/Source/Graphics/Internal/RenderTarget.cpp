#include "RenderTarget.h"

#include "ITexture.h"

namespace Petal {
    std::vector<VkRenderingAttachmentInfo> RenderTarget::CreateColorAttachments() const {
        std::vector<VkRenderingAttachmentInfo> out;
        out.reserve(Colors.size());

        for (const ColorAttachment &attachment : Colors) {
            VkRenderingAttachmentInfo info = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .pNext = nullptr,
                .imageView = attachment.Texture->GetImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .resolveImageView = nullptr,
                .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {.color = {attachment.ClearColor.r, attachment.ClearColor.g, attachment.ClearColor.b, attachment.ClearColor.a}}
            };

            out.push_back(info);
        }

        return out;
    }

    Optional<VkRenderingAttachmentInfo> RenderTarget::CreateDepthAttachment() const {
        if (!Depth) return Result::PETAL_OPTIONAL_EMPTY;

        return VkRenderingAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = Depth->GetImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = nullptr,
            .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue = {.depthStencil = VkClearDepthStencilValue{1.0f, 0}} // todo
        };
    }
} // Petal
