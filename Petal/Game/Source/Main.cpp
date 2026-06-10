#include "GameCamera.h"
#include "Petal.h"
#include "../../Petal/Assets/Shaders/Common.h"
#include "Rendering/Deferred/DeferredGBufferWritePass.h"
#include "Rendering/Deferred/DeferredLightingPass.h"
#include "Rendering/Deferred/DeferredRenderer.h"
using namespace Petal;

// Petal::MeshBuilder CreateMesh() {
//     MeshBuilder mesh({sizeof(VertexData)}, IndexType::INDICES_32_BIT);
//
//     std::array<VertexData, 4> vertices = {
//         VertexData{
//             .position = {-0.5f, -0.5f, 0.0f},
//             .padding = 0,
//             .normal = {0.0f, 0.0f, 1.0f},
//             .padding2 = 0,
//             .uv = {0.0f, 0.0f},
//             .padding3 = 0,
//             .padding4 = 0
//         },
//         VertexData{
//             .position = {-0.5f, 0.5f, 0.0f},
//             .padding = 0,
//             .normal = {0.0f, 0.0f, 1.0f},
//             .padding2 = 0,
//             .uv = {0.0f, 1.0f},
//             .padding3 = 0,
//             .padding4 = 0
//         },
//         VertexData{
//             .position = {0.5f, 0.5f, 0.0f},
//             .padding = 0,
//             .normal = {0.0f, 0.0f, 1.0f},
//             .padding2 = 0,
//             .uv = {1.0f, 1.0f},
//             .padding3 = 0,
//             .padding4 = 0
//         },
//         VertexData{
//             .position = {0.5f, -0.5f, 0.0f},
//             .padding = 0,
//             .normal = {0.0f, 0.0f, 1.0f},
//             .padding2 = 0,
//             .uv = {1.0f, 0.0f},
//             .padding3 = 0,
//             .padding4 = 0
//         }
//     };
//
//     mesh.PushVertices(vertices.size(), vertices.data());
//
//     std::array<glm::u32, 6> indices = {0, 1, 2, 0, 2, 3};
//     mesh.PushIndices<glm::u32>(indices.size(), indices.data());
//
//     return mesh;
// }

Model LoadModel(std::string path, const std::shared_ptr<Logger> &logger, const VertexType &vertexType) {
    Result resultOut;
    Model model(logger, path, {vertexType}, resultOut);
    VertexData v = *static_cast<const VertexData *>(model.GetSections().at(0).Mesh->GetVertices());
    assert(resultOut == Result::SUCCESS);

    logger->Info("Loaded {} model with {} meshes", path, model.GetSections().size());
    return model;
}

class MyPass : public DeferredGBufferWritePass {
public:
    MyPass(
        GraphicsContext &graphicsContext,
        Renderer &renderer,
        DeferredRenderer &deferredRenderer,
        const std::shared_ptr<Logger> &logger,
        const std::shared_ptr<VulkanShader> &shader,
        const std::shared_ptr<ModelResource> &model
    ) : DeferredGBufferWritePass(logger, "MyPass", graphicsContext.GetSwapchain().NumSwapchainImages(), deferredRenderer),
        m_graphicsContext(graphicsContext),
        m_renderer(renderer),
        m_deferredRenderer(deferredRenderer),
        m_model(model) {
        TrackRenderTargetPerCommand(
            deferredRenderer.GetGBuffer(),
            ResourceUsage::ColorAttachment(),
            ResourceUsage::DepthAttachmentReadWrite()
        );
    }

public:
    Result Record(const std::shared_ptr<VulkanSwapchain::RenderCommandBuffers> &renderCommands) const override {
        m_renderer.CmdRender(*renderCommands, m_deferredRenderer.GetGBufferPipeline(), *m_model);

        return Result::SUCCESS;
    }

private:
    GraphicsContext &m_graphicsContext;
    Renderer &m_renderer;
    DeferredRenderer &m_deferredRenderer;
    std::shared_ptr<ModelResource> m_model;
};

int run(Timer &engineShutdownTime) {
    glm::u32 frameNumber = 0;

    Engine engine;
    std::shared_ptr<Logger> logger = engine.GetLoggerSystem().CreateLogger("Game");

    std::shared_ptr<Window> window = engine.GetWindowSystem().OpenWindow();
    OptionalRef<GraphicsContext> contextOptional = engine.GetGraphicsSystem().CreateGraphicsContext(
        window,
        DeviceRequirements::DEFAULT()
    );
    GraphicsContext &graphicsContext = contextOptional.Value();

    // // Render target
    // std::vector<RenderTarget> targets;
    // for (glm::u32 i = 0; i < contextOptional->GetSwapchain().NumSwapchainImages(); i++) {
    //     targets.push_back({
    //         .Colors = {
    //             graphicsContext.GetSwapchain().GetSwapchainRenderTarget()[i].Colors[0],
    //
    //             {
    //                 .Texture = graphicsContext.GetMemorySubsystem().CreateTexture(
    //                     std::format("Render Texture {}", i), {
    //                         .Size = {window->GetDimensions().x, window->GetDimensions().y, 1},
    //                         .ImageType = VK_IMAGE_TYPE_2D,
    //                         .ViewType = VK_IMAGE_VIEW_TYPE_2D,
    //                         .Format = VK_FORMAT_B8G8R8A8_SRGB,
    //                         .Usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    //                         .AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT
    //                     }).Release(),
    //                 .ClearColor = {0, 0, 0, 1}
    //             }
    //         },
    //         .Depth = graphicsContext.GetSwapchain().GetSwapchainRenderTarget()[i].Depth
    //     });
    // }

    // Shader
    ShaderAsset gBufferShaderAsset(
        "C:/Coding/Projects/Petal/Petal/Petal/Assets/Shaders/GBuffer.slang",
        {
            {ShaderType::VERTEX, {"vertexMain"}},
            {ShaderType::FRAGMENT, {"fragmentMain"}}
        }
    );
    std::shared_ptr<VulkanShader> gBufferShader = graphicsContext.CompileShader(gBufferShaderAsset).Release();
    assert(gBufferShader);

    ShaderAsset deferredShaderAsset(
        "C:/Coding/Projects/Petal/Petal/Petal/Assets/Shaders/DeferredLighting.slang",
        {
            {ShaderType::VERTEX, {"vertexMain"}},
            {ShaderType::FRAGMENT, {"fragmentMain"}}
        }
    );
    std::shared_ptr<VulkanShader> deferredShader = graphicsContext.CompileShader(deferredShaderAsset).Release();
    assert(deferredShader);

    // textures
    std::shared_ptr<VulkanTexture> iconTexture = graphicsContext.GetMemorySubsystem().LoadTextureFromDisk(
        "C:/Coding/Projects/Petal/Petal/Petal/Assets/Textures/Icon.png",
        ImageLoaderSettings{},
        TextureCreateInfo{}
    ).Release();
    std::shared_ptr<VulkanTexture> testTexture = graphicsContext.GetMemorySubsystem().LoadTextureFromDisk(
        "C:/Coding/Projects/Petal/Petal/Petal/Assets/Textures/Test.png",
        ImageLoaderSettings{},
        TextureCreateInfo{}
    ).Release();
    gBufferShader->BindTextures("textures", std::vector{iconTexture, testTexture});

    // Renderer
    RendererSettings rendererSettings = {
        .CameraBufferName = "camera"
    };
    std::shared_ptr<Renderer> renderer = graphicsContext.CreateRenderer(rendererSettings).Release();

    // std::shared_ptr<MeshResource> mesh = renderer->UploadMesh(CreateMesh()).Release();
    Model cpuModel = LoadModel("C:/Coding/Projects/Petal/Petal/Petal/Assets/Models/Spider/spider.obj", logger, gBufferShader->GetIntermediateShader().VertexType.Value());
    std::shared_ptr<ModelResource> model = renderer->UploadModel(cpuModel, "MyModel").Release();

    Result result = renderer->Bind(*gBufferShader);
    assert(result == Result::SUCCESS);

    GameCamera camera(window, renderer, logger);

    // Frame graph
    std::shared_ptr<CommandBufferVector> commandBuffers = graphicsContext.CreateCommandBuffers(
        graphicsContext.GetDevice()->GetGraphicsQueueFamily(),
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        graphicsContext.GetSwapchain().NumSwapchainImages()
    ).Release();

    // Deferred
    VulkanGraphicsPipeline::PipelineSettings pipelineSettings = {
        .PushConstantsSize = sizeof(Petal::Params) // todo different settings for different shader?
    };

    DeferredRenderer::RenderPassSupplier<DeferredGBufferWritePass> gBufferWriteSupplier = [&graphicsContext, &renderer, &logger, &gBufferShader, &model
            ](DeferredRenderer &deferredRenderer) {
        return static_cast<std::unique_ptr<DeferredGBufferWritePass>>(std::make_unique<MyPass>(
            graphicsContext, *renderer,
            deferredRenderer,
            logger,
            gBufferShader,
            model
        ));
    };

    DeferredRenderer::RenderPassSupplier<DeferredLightingPass> lightingSupplier = [&logger, &graphicsContext
            ](DeferredRenderer &deferredRenderer) -> AllocatedOptional<DeferredLightingPass> {
        Result deferredLightingResult;
        auto lightingPass = std::make_unique<DeferredLightingPass>(
            logger,
            "Deferred Lighting",
            graphicsContext.GetSwapchain().NumSwapchainImages(),
            deferredRenderer,
            deferredLightingResult
        );
        if (deferredLightingResult == Result::SUCCESS) return lightingPass;
        return deferredLightingResult;
    };

    DeferredRenderer def(
        *renderer,
        engine.GetLoggerSystem().GetLogger(LoggerSystem::GRAPHICS_LOGGER),
        gBufferShader,
        pipelineSettings,
        deferredShader,
        pipelineSettings,
        window->GetDimensions(),
        graphicsContext.GetSwapchain().GetSwapchainRenderTarget(),
        gBufferWriteSupplier,
        lightingSupplier,
        result
    );
    assert(result == Result::SUCCESS);

    std::vector<std::shared_ptr<RenderPass> > passes = def.GetPasses();
    FrameGraph frameGraph(graphicsContext, logger, commandBuffers, std::move(passes), result);
    assert(result == Result::SUCCESS);
    logger->Info("Frame Graph Generated: \n{}", frameGraph.ToString()[0]);

    float dt = FLT_EPSILON;
    while (!window->WantsToClose()) {
        Timer timer;
        frameNumber++;
        glm::u32 swapchainIndex = graphicsContext.GetSwapchain().GetSwapchainIndex();

        engine.Update();
        camera.Update(dt);
        engine.LateUpdate();

        Result result = graphicsContext.GetSwapchain().BeginRendering();
        if (result == Result::PETAL_WINDOW_RESIZED) {
            graphicsContext.RecreateSwapchain();
            frameGraph.RerecordCommandBuffers();
            continue;
        }

        graphicsContext.GetSwapchain().SubmitFrameCommand(CommandBufferStrongRef(commandBuffers, swapchainIndex));

        graphicsContext.GetSwapchain().EndRendering();
        if (result == Result::PETAL_WINDOW_RESIZED) {
            graphicsContext.RecreateSwapchain();
            frameGraph.RerecordCommandBuffers();
            continue;
        }

        engine.Render();
        dt = timer.SecondsSinceStart();
    }
    graphicsContext.DeviceWaitIdle();
    commandBuffers.reset();
    engine.GetWindowSystem().CloseWindow(window);

    engineShutdownTime.Restart();
    return 0;
}


int main() {
    Timer timer;
    int code = run(timer);
    std::cout << "Shut down engine in " << timer.MillisSinceStart() << " ms." << std::endl;
    return code;
}
