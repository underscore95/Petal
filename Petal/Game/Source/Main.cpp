#include "Petal.h"

int main() {
    using namespace Petal;

    glm::u32 frameNumber = 0;

    Engine engine;
    std::shared_ptr<Logger> logger = engine.GetLoggerSystem().CreateLogger("Game");

    std::shared_ptr<Window> window = engine.GetWindowSystem().OpenWindow();
    OptionalRef<Renderer> rendererOptional = engine.GetRenderingSystem().CreateRenderer(
        window,
        DeviceRequirements::DEFAULT()
    );
    if (rendererOptional.IsEmpty()) return -1;

    IntermediateShaderResource out;
    ShaderAsset asset("C:/Coding/Projects/Petal/Petal/Petal/Assets/Shaders/main.slang", ShaderType::FRAGMENT);
    Result r = engine.GetRenderingSystem().GetShaderSubsystem().CompileSlangShader(asset, out);
    logger->Info("created shader: {}", static_cast<int>(r));

    Renderer &renderer = *rendererOptional.Value();
    std::shared_ptr<CommandBufferVector> commandBuffers = renderer.CreateCommandBuffers(
        renderer.GetDevice()->GetGraphicsQueueFamily(),
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        renderer.GetSwapchain().NumSwapchainImages()
    ).Release();

    while (!window->WantsToClose()) {
        frameNumber++;
        glm::u32 swapchainIndex = renderer.GetSwapchain().GetSwapchainIndex();

        engine.Update();
        Result result = renderer.GetSwapchain().BeginRendering();
        if (result == Result::PETAL_WINDOW_RESIZED) {
            renderer.RecreateSwapchain();
            continue;
        }

        commandBuffers->Begin(swapchainIndex, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        renderer.GetSwapchain().CmdClear(
            commandBuffers->GetHandle(swapchainIndex),
            Color{0.0f, 0.0f, (frameNumber % 10000) / 10000.0f, 1.0f},
            swapchainIndex
        );

        result = commandBuffers->End(swapchainIndex);

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
