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
        std::vector<std::unique_ptr<RenderPass> > passes,
        Result &resultOut
    )
        : m_context(context),
          m_logger(logger),
          m_commands(commands),
          m_passes(std::move(passes)) {
        resultOut = Record();

        if (m_passes.empty()) {
            m_logger->Warn("Frame graph created with 0 render passes");
        }
    }

    Result FrameGraph::RerecordCommandBuffers() {
        for (auto &pair : m_resources) {
            Optional<ResourceState> &state = pair.second;
            state = Result::PETAL_OPTIONAL_EMPTY;
        }

        return Record();
    }

    const std::vector<std::string> &FrameGraph::ToString() const {
        return m_visualRepresentation;
    }

    Result FrameGraph::PushBarriers(
        Optional<ResourceState> &state,
        const RenderPass::PassResource &resource,
        std::vector<VkBufferMemoryBarrier2> &bufferBarriers,
        std::vector<VkImageMemoryBarrier2> &imageBarriers,
        std::string &graphVisualRepresentation
    ) {
        if (state.HasValue()) {
            bool isParallelRead = state->LastAccess == ResourceAccess::READ && resource.AccessType == ResourceAccess::READ;
            bool isImageTransition = resource.RequiredImageLayout.HasValue() && resource.RequiredImageLayout != state->LastImageLayout;
            if (isParallelRead && !isImageTransition) {
                // Parallel reads don't require sync
                return Result::SUCCESS;
            }
        }

        VkAccessFlagBits2 previousAccess = 0;
        if (state.HasValue()) {
            if (ResourceAccesses::GetData(state->LastAccess).IsRead) previousAccess |= VK_ACCESS_2_MEMORY_READ_BIT;
            if (ResourceAccesses::GetData(state->LastAccess).IsWrite) previousAccess |= VK_ACCESS_2_MEMORY_WRITE_BIT;
        }

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

            graphVisualRepresentation += std::format(
                "Buffer Barrier on {}"
                "\n  Previous Access {}"
                "\n  New Access {}\n\n",
                resource.Resource->GetName(),
                string_VkAccessFlags2(barrier.srcAccessMask),
                string_VkAccessFlags2(barrier.dstAccessMask)
            );
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
                .oldLayout = state.HasValue() && state->LastImageLayout.HasValue() ? state->LastImageLayout.Value() : VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = *resource.RequiredImageLayout,
                .srcQueueFamilyIndex = queueIndex,
                .dstQueueFamilyIndex = queueIndex,
                .image = texture->GetImage(),
                .subresourceRange = texture->GetRange()
            };
            imageBarriers.push_back(barrier);

            graphVisualRepresentation += std::format(
                "Texture Barrier on {}"
                "\n  Layout {} to {}"
                "\n  Previous Access {}"
                "\n  New Access {}\n\n",
                resource.Resource->GetName(),
                barrier.oldLayout, barrier.newLayout,
                string_VkAccessFlags2(barrier.srcAccessMask),
                string_VkAccessFlags2(barrier.dstAccessMask)
            );
        } else {
            PETAL_ERROR(Result::FRAME_GRAPH_ERROR, m_logger, "Invalid resource category {}", ResourceTypes::GetData( resource.ResourceType ).ResourceCategory);
        }

        return Result::SUCCESS;
    }

    Result FrameGraph::Record() {
        m_visualRepresentation.clear();
        m_visualRepresentation.resize(m_commands->Size());

        Result result = m_commands->BeginAll(0);
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        for (std::unique_ptr<RenderPass> &pass : m_passes) {
            PETAL_CHECK_COND(pass == nullptr, Result::FRAME_GRAPH_ERROR, m_logger, "Null pass in frame graph");
            PETAL_CHECK_COND(
                pass->GetNumCommandBuffers() != m_commands->Size(),
                Result::FRAME_GRAPH_ERROR,
                m_logger,
                "Render pass expects {} command buffers but frame graph has {}", pass->GetNumCommandBuffers(), m_commands->Size()
            );

            for (glm::u32 commandIndex = 0; commandIndex < m_commands->Size(); commandIndex++) {
                std::vector<VkBufferMemoryBarrier2> bufferBarriers;
                std::vector<VkImageMemoryBarrier2> imageBarriers;
                for (const RenderPass::PassResource &resource : pass->GetAccessedResources()[commandIndex]) {
                    auto [it,_] = m_resources.try_emplace(resource.Resource, Result::PETAL_OPTIONAL_EMPTY);
                    Optional<ResourceState> &state = it->second;

                    result = PushBarriers(state, resource, bufferBarriers, imageBarriers, m_visualRepresentation[commandIndex]);
                    PETAL_CHECK_COND(result != Result::SUCCESS, result, m_logger, "Failed to insert barriers for render pass {}", pass->GetName());

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

                vkCmdPipelineBarrier2(m_commands->GetHandle(commandIndex), &depInfo);

                result = pass->Record(m_commands);
                PETAL_CHECK_COND(result != Result::SUCCESS, result, m_logger, "Failed to record commands for render pass {}", pass->GetName());

                m_visualRepresentation[commandIndex] += "Executed RenderPass: " + pass->GetName() + "\n\n";
            }
        }

        result = m_commands->EndAll();
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        return Result::SUCCESS;
    }
} // Petal
