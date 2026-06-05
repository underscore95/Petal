#include "ResourceUsage.h"

namespace Petal {
    ResourceUsage::ResourceUsage(
        VkPipelineStageFlags2 stageMask,
        VkAccessFlags2 accessMask,
        Optional<VkImageLayout> imageLayout
    )
        : StageMask(stageMask),
          AccessMask(accessMask),
          ImageLayout(imageLayout) {
    }

    ResourceUsage ResourceUsage::Undefined() {
        return {
            VK_PIPELINE_STAGE_2_NONE,
            VK_ACCESS_2_NONE,
            VK_IMAGE_LAYOUT_UNDEFINED
        };
    }

    ResourceUsage ResourceUsage::ColorAttachment() {
        return {
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };
    }

    ResourceUsage ResourceUsage::DepthAttachmentRead() {
        return {
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
            VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
            VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL
        };
    }

    ResourceUsage ResourceUsage::DepthAttachmentWrite() {
        return {
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
            VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
        };
    }

    ResourceUsage ResourceUsage::DepthAttachmentReadWrite() {
        return {
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
            VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
        };
    }

    ResourceUsage ResourceUsage::Sampled(
        VkPipelineStageFlags2 shaderStages
    ) {
        return {
            shaderStages,
            VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
    }

    ResourceUsage ResourceUsage::StorageRead(
        VkPipelineStageFlags2 shaderStages
    ) {
        return {
            shaderStages,
            VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
            VK_IMAGE_LAYOUT_GENERAL
        };
    }

    ResourceUsage ResourceUsage::StorageWrite(
        VkPipelineStageFlags2 shaderStages
    ) {
        return {
            shaderStages,
            VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
            VK_IMAGE_LAYOUT_GENERAL
        };
    }

    ResourceUsage ResourceUsage::StorageReadWrite(
        VkPipelineStageFlags2 shaderStages
    ) {
        return {
            shaderStages,
            VK_ACCESS_2_SHADER_STORAGE_READ_BIT |
            VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
            VK_IMAGE_LAYOUT_GENERAL
        };
    }

    ResourceUsage ResourceUsage::UniformBuffer(
        VkPipelineStageFlags2 shaderStages
    ) {
        return {
            shaderStages,
            VK_ACCESS_2_UNIFORM_READ_BIT
        };
    }

    ResourceUsage ResourceUsage::VertexBuffer() {
        return {
            VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT,
            VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT
        };
    }

    ResourceUsage ResourceUsage::IndexBuffer() {
        return {
            VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT,
            VK_ACCESS_2_INDEX_READ_BIT
        };
    }

    ResourceUsage ResourceUsage::IndirectBuffer() {
        return {
            VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT,
            VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT
        };
    }

    ResourceUsage ResourceUsage::TransferSource() {
        return {
            VK_PIPELINE_STAGE_2_COPY_BIT,
            VK_ACCESS_2_TRANSFER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
        };
    }

    ResourceUsage ResourceUsage::TransferDestination() {
        return {
            VK_PIPELINE_STAGE_2_COPY_BIT,
            VK_ACCESS_2_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        };
    }

    ResourceUsage ResourceUsage::Present() {
        return {
            VK_PIPELINE_STAGE_2_NONE,
            VK_ACCESS_2_NONE,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };
    }
}
