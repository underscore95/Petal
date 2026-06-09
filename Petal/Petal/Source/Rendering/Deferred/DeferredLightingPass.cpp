#include "DeferredLightingPass.h"

#include "DeferredRenderer.h"
#include "../../../Assets/Shaders/Common.h"
#include "Graphics/Internal/VulkanShader.h"
#include "Graphics/Other/IndexType.h"
#include "Rendering/Renderer.h"
#include "Resources/MeshBuilder.h"

namespace Petal {
    DeferredLightingPass::DeferredLightingPass(
        const std::shared_ptr<Logger> &logger,
        const std::string &name,
        glm::u32 numCommandBuffers,
        DeferredRenderer &deferredRenderer,
        Result &resultOut
    ) : RenderPass(logger, name, numCommandBuffers),
        m_deferredRenderer(deferredRenderer) {
        resultOut = CreateQuadMesh();
        if (resultOut != Result::SUCCESS) return;

        // bind g buffer
        // todo support different descriptor sets for different command buffers
        resultOut = m_deferredRenderer.GetLightingPipeline().GetShader().BindTexture("gBufferAlbedo", *deferredRenderer.GetGBufferTextures()[0].Albedo);
        resultOut = m_deferredRenderer.GetLightingPipeline().GetShader().BindTexture("gBufferNormal", *deferredRenderer.GetGBufferTextures()[0].Normal);
        resultOut = m_deferredRenderer.GetLightingPipeline().GetShader().BindTexture("gBufferPosition", *deferredRenderer.GetGBufferTextures()[0].Position);
        if (resultOut != Result::SUCCESS) return;

        // g buffer
        TrackRenderTargetPerCommand(
            m_deferredRenderer.GetGBuffer(),
            ResourceUsage::Sampled(VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT),
            ResourceUsage::Invalid()
        );

        // our target
        TrackRenderTargetPerCommand(
            m_deferredRenderer.GetFinalRenderTarget(),
            ResourceUsage::ColorAttachment(),
            ResourceUsage::DepthAttachmentReadWrite()
        );
    }

    Result DeferredLightingPass::Record(const std::shared_ptr<CommandBufferVector> &commands) const {
        m_deferredRenderer.GetLightingPipeline().GetShader().CmdBindResources(m_deferredRenderer.GetLightingPipeline(), *commands);

        std::shared_ptr<VulkanSwapchain::RenderCommandBuffers> renderCommands = m_deferredRenderer.GetContext().GetSwapchain().CmdBeginRendering(
            commands,
            m_deferredRenderer.GetFinalRenderTarget()
        );

        Params params = {}; // todo don't hard code
        m_deferredRenderer.GetRenderer().CmdRender(*renderCommands, m_deferredRenderer.GetLightingPipeline(), params, *m_quadMesh);

        return Result::SUCCESS;
    }

    Result DeferredLightingPass::CreateQuadMesh() {
        const Optional<VertexType> &vertexType = m_deferredRenderer.GetLightingPipeline().GetShader().GetIntermediateShader().VertexType;
        PETAL_CHECK_OPTIONAL(vertexType, m_logger, "Deferred lighting shader has no vertex type");
        MeshBuilder mesh(vertexType.Value(), IndexType::INDICES_16_BIT);

        // todo don't hard code?
        constexpr std::array<VertexData, 3> VERTICES = {
            VertexData{
                .position = {-1.0f, -1.0f, 0.0f},
                .normal = {0.0f, 0.0f, 1.0f},
                .uv = {0.0f, 0.0f}
            },
            VertexData{
                .position = {-1.0f, 3.0f, 0.0f},
                .normal = {0.0f, 0.0f, 1.0f},
                .uv = {0.0f, 2.0f}
            },
            VertexData{
                .position = {3.0f, -1.0f, 0.0f},
                .normal = {0.0f, 0.0f, 1.0f},
                .uv = {2.0f, 0.0f}
            }
        };

        mesh.PushVertices(VERTICES.size(), VERTICES.data());

        constexpr std::array<glm::u16, 3> INDICES = {0, 1, 2};
        mesh.SetIndices(INDICES.data(), INDICES.size() * sizeof(glm::u16), IndexType::INDICES_16_BIT);

        AllocatedOptional<MeshResource> uploadedMesh = m_deferredRenderer.GetRenderer().UploadMesh(mesh, "Deferred Quad");
        PETAL_CHECK_OPTIONAL(uploadedMesh, m_logger, "Failed to upload deferred quad mesh");

        m_quadMesh = uploadedMesh.Release();
        return Result::SUCCESS;
    }
} // Petal
