#include "FrameGraph.h"

#include "RenderPass.h"
#include "Graphics/Internal/ITexture.h"
#include "Graphics/Internal/RenderingDevice.h"
#include "Graphics/Internal/VulkanQueue.h"
#include "Graphics/Internal/CommandBuffers/CommandBufferVector.h"
#include "Graphics/Memory/Buffers/IBuffer.h"

namespace Petal {
    FrameGraph::FrameGraph(
        GraphicsContext &context,
        const std::shared_ptr<Logger> &logger,
        const std::shared_ptr<CommandBufferVector> &commands,
        const std::vector<std::shared_ptr<IVulkanResource> > &resources,
        std::vector<std::unique_ptr<RenderPass> > &&passes,
        Result &resultOut
    )
        : m_context(context),
          m_logger(logger),
          m_commands(commands),
          m_passes(std::move(passes)) {
        for (const std::shared_ptr<IVulkanResource> &resource : resources) {
            m_resources.emplace(resource, Result::PETAL_OPTIONAL_EMPTY);
        }

        resultOut = Record();
    }

    Result FrameGraph::RerecordCommandBuffers() {
        for (auto &pair : m_resources) {
            Optional<ResourceState> &state = pair.second;
            state = Result::PETAL_OPTIONAL_EMPTY;
        }

        return Record();
    }

    Result FrameGraph::PushBarriers(
        const ResourceState &state,
        const RenderPass::PassResource &resource,
        std::vector<VkBufferMemoryBarrier2> &bufferBarriers,
        std::vector<VkImageMemoryBarrier2> &imageBarriers
    ) {
        if (state.LastAccess == ResourceAccess::READ && resource.AccessType == ResourceAccess::READ) {
            // Parallel reads are okay
            return Result::SUCCESS;
        }

        VkAccessFlagBits2 previousAccess = 0;
        if (ResourceAccesses::GetData(state.LastAccess).IsRead) previousAccess |= VK_ACCESS_2_MEMORY_READ_BIT;
        if (ResourceAccesses::GetData(state.LastAccess).IsWrite) previousAccess |= VK_ACCESS_2_MEMORY_WRITE_BIT;

        VkAccessFlagBits2 newAccess = 0;
        if (ResourceAccesses::GetData(resource.AccessType).IsRead) newAccess |= VK_ACCESS_2_MEMORY_READ_BIT;
        if (ResourceAccesses::GetData(resource.AccessType).IsWrite) newAccess |= VK_ACCESS_2_MEMORY_WRITE_BIT;

        // todo queue
        glm::u32 queueIndex = m_context.GetDevice()->GetGraphicsQueueFamily().GetQueueFamilyIndex();

        ResourceTypes::Category category = ResourceTypes::GetData(resource.ResourceType).ResourceCategory;
        if (category == ResourceTypes::Category::BUFFER) {
            const auto buffer = dynamic_cast<const IBuffer *>(resource.Resource.get());
            PETAL_CHECK_COND(buffer == nullptr, Result::FRAME_GRAPH_ERROR, m_logger, "Failed to cast resource {} to IBuffer", resource.Resource->GetName());
            VkBufferMemoryBarrier2 barrier = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                .srcAccessMask = previousAccess,
                .dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                .dstAccessMask = newAccess,
                .srcQueueFamilyIndex = queueIndex,
                .dstQueueFamilyIndex = queueIndex,
                .buffer = buffer->GetHandle(),
                .offset = buffer->GetOffset(),
                .size = buffer->GetSize()
            };
            bufferBarriers.push_back(barrier);
        } else if (category == ResourceTypes::Category::TEXTURE) {
            const auto texture = dynamic_cast<const ITexture *>(resource.Resource.get());
            PETAL_CHECK_COND(texture == nullptr, Result::FRAME_GRAPH_ERROR, m_logger, "Failed to cast resource {} to ITexture", resource.Resource->GetName());
            PETAL_CHECK_OPTIONAL(resource.RequiredImageLayout, m_logger, "Missing image layout in pass resource {}", resource.Resource->GetName());

            VkImageMemoryBarrier2 barrier = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                .srcAccessMask = previousAccess,
                .dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                .dstAccessMask = newAccess,
                .oldLayout = state.LastImageLayout.OrElse(VK_IMAGE_LAYOUT_UNDEFINED),
                .newLayout = *resource.RequiredImageLayout,
                .srcQueueFamilyIndex = queueIndex,
                .dstQueueFamilyIndex = queueIndex,
                .image = texture->GetImage(),
                .subresourceRange = texture->GetRange()
            };
            imageBarriers.push_back(barrier);
        } else {
            PETAL_ERROR(Result::FRAME_GRAPH_ERROR, m_logger, "Invalid resource category {}", ResourceTypes::GetData( resource.ResourceType ).ResourceCategory);
        }

        return Result::SUCCESS;
    }

    Result FrameGraph::Record() {
        Result result;

        for (std::unique_ptr<RenderPass> &pass : m_passes) {
            assert(pass);

            std::vector<VkBufferMemoryBarrier2> bufferBarriers;
            std::vector<VkImageMemoryBarrier2> imageBarriers;

            for (const RenderPass::PassResource &resource : pass->GetAccessedResources()) {
                auto it = m_resources.find(resource.Resource);
                PETAL_CHECK_COND(
                    it == m_resources.end(),
                    Result::FRAME_GRAPH_ERROR,
                    m_logger,
                    "Pass {} referenced resource {} which the frame graph didn't know about", pass->GetName(), resource.Resource->GetName()
                );
                Optional<ResourceState> &state = it->second;

                if (state.HasValue()) {
                    result = PushBarriers(*state, resource, bufferBarriers, imageBarriers);
                    PETAL_CHECK_COND(result != Result::SUCCESS, result, m_logger, "Failed to insert barriers for render pass {}", pass->GetName());
                }

                state = ResourceState{
                    .LastPass = pass.get(),
                    .LastAccess = resource.AccessType,
                    .LastImageLayout = resource.RequiredImageLayout
                };
            }

            VkDependencyInfo depInfo{
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .pNext = nullptr,
                .dependencyFlags = 0,
                .memoryBarrierCount = 0,
                .pMemoryBarriers = nullptr,
                .bufferMemoryBarrierCount = static_cast<glm::u32>(bufferBarriers.size()),
                .pBufferMemoryBarriers = bufferBarriers.empty() ? nullptr : bufferBarriers.data(),
                .imageMemoryBarrierCount = static_cast<glm::u32>(imageBarriers.size()),
                .pImageMemoryBarriers = imageBarriers.empty() ? nullptr : imageBarriers.data()
            };
            for (glm::u32 i = 0; i < m_commands->Size(); i++) {
                vkCmdPipelineBarrier2(m_commands->GetHandle(i), &depInfo);
            }

            result = pass->Record(m_commands);
            PETAL_CHECK_COND(result != Result::SUCCESS, result, m_logger, "Failed to record commands for render pass {}", pass->GetName());
        }

        return Result::SUCCESS;
    }
} // Petal
