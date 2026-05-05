#include "Petal.h"
#include "Graphics/Memory/GPUBufferSubsystem.h"
#include "../../Petal/Assets/Shaders/Common.h"

int main() {
    using namespace Petal;

    glm::u32 frameNumber = 0;

    Engine engine;
    std::shared_ptr<Logger> logger = engine.GetLoggerSystem().CreateLogger("Game");

    std::shared_ptr<Window> window = engine.GetWindowSystem().OpenWindow();
    OptionalRef<GraphicsContext> rendererOptional = engine.GetRenderingSystem().CreateRenderer(
        window,
        DeviceRequirements::DEFAULT()
    );
    if (rendererOptional.IsEmpty()) return -1;

    GraphicsContext &renderer = *rendererOptional.Value();
    std::shared_ptr<CommandBufferVector> commandBuffers = renderer.CreateCommandBuffers(
        renderer.GetDevice()->GetGraphicsQueueFamily(),
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        renderer.GetSwapchain().NumSwapchainImages()
    ).Release();

    IntermediateShaderResource out;
    ShaderAsset asset(
        "C:/Coding/Projects/Petal/Petal/Petal/Assets/Shaders/main.slang",
        {
            {ShaderType::VERTEX, {"vertexMain"}},
            {ShaderType::FRAGMENT, {"fragmentMain"}}
        }
    );
    auto shaderOpt = renderer.CompileShader(asset);

    // Buffers
    using namespace PetalShader;
    std::array<VertexData, 3> vertexData = {
        VertexData{{0.0f, 0.5f, 0.0f}, 0, {0.0f, 0.0f, 1.0f}, 0, {0.5f, 1.0f}, 0, 0},
        VertexData{{0.5f, -0.5f, 0.0f}, 0, {0.0f, 0.0f, 1.0f}, 0, {1.0f, 0.0f}, 0, 0},
        VertexData{{-0.5f, -0.5f, 0.0f}, 0, {0.0f, 0.0f, 1.0f}, 0, {0.0f, 0.0f}, 0, 0}
    };

    std::array<glm::u32, 3> indexData = {0, 1, 2};

    // Create buffers
    auto vertexBufferOptional =
            renderer.GetBufferSubsystem().CreateIndependentBuffer(
                "Vertex Buffer",
                sizeof(VertexData) * vertexData.size()
            );
    assert(vertexBufferOptional.HasValue());
    std::shared_ptr<GPUBuffer> vertexBuffer = vertexBufferOptional.Release();
    shaderOpt->BindBuffer("vertices", *vertexBuffer);

    auto indexBufferOptional =
            renderer.GetBufferSubsystem().CreateIndependentBuffer(
                "Index Buffer",
                sizeof(glm::u32) * indexData.size()
            );
    assert(indexBufferOptional.HasValue());
    std::shared_ptr<GPUBuffer> indexBuffer = indexBufferOptional.Release();
    shaderOpt->BindBuffer("indices", *indexBuffer);

    // Upload buffer data
    Result result;

    result = renderer.GetBufferSubsystem().Write(
        *vertexBuffer,
        vertexData.data(),
        sizeof(vertexData[0]) * vertexData.size()
    );
    assert(result == Result::SUCCESS);

    result = renderer.GetBufferSubsystem().Write(
        *indexBuffer,
        indexData.data(),
        sizeof(indexData[0]) * indexData.size()
    );
    assert(result == Result::SUCCESS);

    commandBuffers->BeginAll(0);

    renderer.GetSwapchain().CmdBeginRendering(*commandBuffers);

    for (glm::u32 swapchainIndex = 0; swapchainIndex < commandBuffers->Size(); swapchainIndex++) {
        shaderOpt->BindResources(commandBuffers->GetHandle(swapchainIndex));
        // renderer.GetSwapchain().CmdClear(
        //     commandBuffers->GetHandle(swapchainIndex),
        //     Color{0.0f, 0.0f, (frameNumber % 10000) / 10000.0f, 1.0f},
        //     swapchainIndex
        // );
    }

    renderer.GetSwapchain().CmdRender(*shaderOpt.Value(), *commandBuffers, indexData.size());
    renderer.GetSwapchain().CmdEndRendering(*commandBuffers);

    commandBuffers->EndAll();

    while (!window->WantsToClose()) {
        frameNumber++;
        glm::u32 swapchainIndex = renderer.GetSwapchain().GetSwapchainIndex();

        engine.Update();
        Result result = renderer.GetSwapchain().BeginRendering();
        if (result == Result::PETAL_WINDOW_RESIZED) {
            renderer.RecreateSwapchain();
            continue;
        }

        renderer.GetSwapchain().SubmitFrameCommand(CommandBufferStrongRef(commandBuffers, swapchainIndex));

        engine.Render();
        renderer.GetSwapchain().EndRendering();
        if (result == Result::PETAL_WINDOW_RESIZED) {
            renderer.RecreateSwapchain();
            continue;
        }
    }
    renderer.DeviceWaitIdle();
    commandBuffers.reset();
    engine.GetWindowSystem().CloseWindow(window);
    return 0;
}
