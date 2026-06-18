#include "FrameGraph.h"

#include "RenderPass.h"
#include "Graphics/Internal/ITexture.h"
#include "Graphics/Internal/RenderingDevice.h"
#include "Graphics/Internal/VulkanQueue.h"
#include "Graphics/Internal/CommandBuffers/CommandBufferVector.h"
#include "Graphics/Memory/Buffers/IBuffer.h"

namespace Petal {
    FrameGraph::ResourceState::ResourceState()
        : LastPass(nullptr),
          LastUsage(VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_NONE) {
    }

    FrameGraph::ResourceState::ResourceState(
        const RenderPass &lastPass,
        const ResourceUsage &lastUsage
    ) : LastPass(&lastPass),
        LastUsage(lastUsage) {
    }

    FrameGraph::FrameGraph(
        GraphicsContext &context,
        const std::shared_ptr<Logger> &logger,
        const std::shared_ptr<CommandBufferVector> &commands,
        std::vector<std::shared_ptr<RenderPass> > passes,
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
            assert(state->LastPass);

            bool isParallelRead = IsRead(state->LastUsage.AccessMask)
                                  && !IsWrite(state->LastUsage.AccessMask)
                                  && IsRead(resource.Usage.AccessMask)
                                  && !IsWrite(resource.Usage.AccessMask);
            bool isImageTransition = resource.Usage.ImageLayout.HasValue() && resource.Usage.ImageLayout != state->LastUsage.ImageLayout;
            if (isParallelRead && !isImageTransition) {
                // Parallel reads don't require sync
                return Result::SUCCESS;
            }
        }

        // todo queue
        glm::u32 queueIndex = m_context.GetDevice()->GetGraphicsQueueFamily().GetQueueFamilyIndex();
        const VkAccessFlagBits2 lastAccess = state.HasValue() ? state->LastUsage.AccessMask : VK_ACCESS_2_NONE;
        VkPipelineStageFlagBits2 lastStage = state.HasValue() ? state->LastUsage.StageMask : VK_PIPELINE_STAGE_2_NONE;

        ResourceTypes::Category category = ResourceTypes::GetData(resource.ResourceType).ResourceCategory;
        if (category == ResourceTypes::Category::BUFFER) {
            const auto buffer = dynamic_cast<const IBuffer *>(resource.Resource.get());
            PETAL_CHECK_COND(buffer == nullptr, Result::FRAME_GRAPH_ERROR, m_logger, "Failed to cast resource {} to IBuffer", resource.Resource->GetName());
            VkBufferMemoryBarrier2 barrier = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = lastStage,
                .srcAccessMask = lastAccess,
                .dstStageMask = resource.Usage.StageMask,
                .dstAccessMask = resource.Usage.AccessMask,
                .srcQueueFamilyIndex = queueIndex,
                .dstQueueFamilyIndex = queueIndex,
                .buffer = buffer->GetHandle(),
                .offset = buffer->GetOffset(),
                .size = buffer->GetSize()
            };
            bufferBarriers.push_back(barrier);

            graphVisualRepresentation += std::format(
                "Buffer Barrier on {}"
                "\n  Previous Stage {}"
                "\n  New Stage {}"
                "\n  Previous Access {}"
                "\n  New Access {}\n\n",
                resource.Resource->GetName(),
                string_VkPipelineStageFlags2(barrier.srcStageMask),
                string_VkPipelineStageFlags2(barrier.dstStageMask),
                string_VkAccessFlags2(barrier.srcAccessMask),
                string_VkAccessFlags2(barrier.dstAccessMask)
            );
        } else if (category == ResourceTypes::Category::TEXTURE) {
            const auto texture = dynamic_cast<const ITexture *>(resource.Resource.get());
            PETAL_CHECK_COND(texture == nullptr, Result::FRAME_GRAPH_ERROR, m_logger, "Failed to cast resource {} to ITexture", resource.Resource->GetName());
            PETAL_CHECK_OPTIONAL(resource.Usage.ImageLayout, m_logger, "Missing image layout in pass resource {}", resource.Resource->GetName());

            if (state.IsEmpty() && texture->IsSwapchainImage()) {
                // Need a stronger synchronization since the swapchain owns this resource, not us
                lastStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            }

            VkImageMemoryBarrier2 barrier = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = lastStage,
                .srcAccessMask = lastAccess,
                .dstStageMask = resource.Usage.StageMask,
                .dstAccessMask = resource.Usage.AccessMask,
                .oldLayout = state.HasValue() && state->LastUsage.ImageLayout.HasValue() ? state->LastUsage.ImageLayout.Value() : VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = *resource.Usage.ImageLayout,
                .srcQueueFamilyIndex = queueIndex,
                .dstQueueFamilyIndex = queueIndex,
                .image = texture->GetImage(),
                .subresourceRange = texture->GetRange()
            };
            imageBarriers.push_back(barrier);

            graphVisualRepresentation += std::format(
                "Texture Barrier on {}"
                "\n  Layout {} to {}"
                "\n  Previous Stage {}"
                "\n  New Stage {}"
                "\n  Previous Access {}"
                "\n  New Access {}\n\n",
                resource.Resource->GetName(),
                barrier.oldLayout, barrier.newLayout,
                string_VkPipelineStageFlags2(barrier.srcStageMask),
                string_VkPipelineStageFlags2(barrier.dstStageMask),
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

        for (std::shared_ptr<RenderPass> &pass : m_passes) {
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

                    state = ResourceState{*pass, resource.Usage};
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

                m_visualRepresentation[commandIndex] += "Executed RenderPass: " + pass->GetName() + "\n\n";
            }

            result = pass->Record(m_commands);
            PETAL_CHECK_COND(result != Result::SUCCESS, result, m_logger, "Failed to record commands for render pass {}", pass->GetName());
        }

        result = m_commands->EndAll();
        PETAL_CHECK_COND_SILENT(result != Result::SUCCESS, result);

        return Result::SUCCESS;
    }
} // Petal
