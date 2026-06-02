#include "RenderTarget.h"

#include "IHasVulkanImage.h"

namespace Petal {
    Optional<VkRenderingAttachmentInfo> RenderTarget::CreateColorAttachment() const {
        if (!Color) return Result::PETAL_OPTIONAL_EMPTY;

        return VkRenderingAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = Color->GetImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = nullptr,
            .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {.color = {0, 0, 0, 1}} // todo
        };
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
