#include "Petal.h"
#include "../../Petal/Assets/Shaders/Common.h"
using namespace Petal;
using namespace PetalShader;

Petal::MeshBuilder CreateMesh() {
    MeshBuilder mesh({sizeof(VertexData)}, IndexType::INDICES_32_BIT);

    VertexData vertices[4] = {
        {
            .position = {-0.5f, -0.5f, 0.0f},
            .padding = 0,
            .normal = {0.0f, 0.0f, 1.0f},
            .padding2 = 0,
            .uv = {0.0f, 0.0f},
            .padding3 = 0,
            .padding4 = 0
        },
        {
            .position = {-0.5f, 0.5f, 0.0f},
            .padding = 0,
            .normal = {0.0f, 0.0f, 1.0f},
            .padding2 = 0,
            .uv = {0.0f, 1.0f},
            .padding3 = 0,
            .padding4 = 0
        },
        {
            .position = {0.5f, 0.5f, 0.0f},
            .padding = 0,
            .normal = {0.0f, 0.0f, 1.0f},
            .padding2 = 0,
            .uv = {1.0f, 1.0f},
            .padding3 = 0,
            .padding4 = 0
        },
        {
            .position = {0.5f, -0.5f, 0.0f},
            .padding = 0,
            .normal = {0.0f, 0.0f, 1.0f},
            .padding2 = 0,
            .uv = {1.0f, 0.0f},
            .padding3 = 0,
            .padding4 = 0
        }
    };

    mesh.PushVertex(&vertices[0]);
    mesh.PushVertex(&vertices[1]);
    mesh.PushVertex(&vertices[2]);
    mesh.PushVertex(&vertices[3]);

    mesh.PushIndex<glm::u32>(0);
    mesh.PushIndex<glm::u32>(1);
    mesh.PushIndex<glm::u32>(2);

    mesh.PushIndex<glm::u32>(0);
    mesh.PushIndex<glm::u32>(2);
    mesh.PushIndex<glm::u32>(3);

    return mesh;
}

void RecordCommandBuffers(GraphicsContext &graphicsContext, std::shared_ptr<CommandBufferVector> commandBuffers, std::shared_ptr<VulkanShader> shader,
                          std::shared_ptr<MeshResource> mesh) {
    commandBuffers->BeginAll(0);

    graphicsContext.GetSwapchain().CmdBeginRendering(*commandBuffers);

    shader->BindResources(*commandBuffers);

    graphicsContext.GetSwapchain().CmdRender(*shader, *commandBuffers, mesh->GetNumIndices());
    graphicsContext.GetSwapchain().CmdEndRendering(*commandBuffers);

    commandBuffers->EndAll();
}

int main() {
    glm::u32 frameNumber = 0;

    Engine engine;
    std::shared_ptr<Logger> logger = engine.GetLoggerSystem().CreateLogger("Game");

    std::shared_ptr<Window> window = engine.GetWindowSystem().OpenWindow();
    OptionalRef<GraphicsContext> rendererOptional = engine.GetGraphicsSystem().CreateGraphicsContext(
        window,
        DeviceRequirements::DEFAULT()
    );
    if (rendererOptional.IsEmpty()) return -1;

    GraphicsContext &graphicsContext = *rendererOptional.Value();
    std::shared_ptr<CommandBufferVector> commandBuffers = graphicsContext.CreateCommandBuffers(
        graphicsContext.GetDevice()->GetGraphicsQueueFamily(),
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        graphicsContext.GetSwapchain().NumSwapchainImages()
    ).Release();

    IntermediateShaderResource out;
    ShaderAsset asset(
        "C:/Coding/Projects/Petal/Petal/Petal/Assets/Shaders/main.slang",
        {
            {ShaderType::VERTEX, {"vertexMain"}},
            {ShaderType::FRAGMENT, {"fragmentMain"}}
        }
    );
    std::shared_ptr<VulkanShader> shader = graphicsContext.CompileShader(asset).Release();

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
    shader->BindTextures("textures", {iconTexture, testTexture});

    // Renderer
    RendererSettings rendererSettings = {
        .VertexBufferShaderName = "vertices",
        .IndexBufferShaderName = "indices"
    };
    auto renderer = graphicsContext.CreateRenderer(rendererSettings).Release();

    std::shared_ptr<MeshResource> mesh = renderer->UploadMesh(CreateMesh()).Release();

    renderer->Bind(*shader);

    RecordCommandBuffers(graphicsContext, commandBuffers, shader, mesh);

    while (!window->WantsToClose()) {
        frameNumber++;
        glm::u32 swapchainIndex = graphicsContext.GetSwapchain().GetSwapchainIndex();

        engine.Update();
        Result result = graphicsContext.GetSwapchain().BeginRendering();
        if (result == Result::PETAL_WINDOW_RESIZED) {
            graphicsContext.RecreateSwapchain();
            RecordCommandBuffers(graphicsContext, commandBuffers, shader, mesh);
            continue;
        }

        graphicsContext.GetSwapchain().SubmitFrameCommand(CommandBufferStrongRef(commandBuffers, swapchainIndex));

        graphicsContext.GetSwapchain().EndRendering();
        if (result == Result::PETAL_WINDOW_RESIZED) {
            graphicsContext.RecreateSwapchain();
            RecordCommandBuffers(graphicsContext, commandBuffers, shader, mesh);
            continue;
        }

        engine.Render();
    }
    graphicsContext.DeviceWaitIdle();
    commandBuffers.reset();
    engine.GetWindowSystem().CloseWindow(window);
    return 0;
}
