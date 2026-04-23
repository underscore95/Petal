#include "Petal.h"

int main() {
    using namespace Petal;

    glm::u32 frameNumber = 0;

    Engine engine;
    std::shared_ptr<Window> window = engine.GetWindowSystem().OpenWindow();
    OptionalRef<Renderer> rendererOptional = engine.GetRenderingSystem().CreateRenderer(
        window,
        DeviceRequirements::DEFAULT()
    );
    if (rendererOptional.IsEmpty()) return -1;

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
        renderer.GetSwapchain().BeginRendering();

        commandBuffers->Begin(swapchainIndex, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        renderer.GetSwapchain().CmdClear(
            commandBuffers->GetHandle(swapchainIndex),
            Color{0.0f, 0.0f, (frameNumber % 10000) / 10000.0f, 1.0f},
            swapchainIndex
        );

        commandBuffers->End(swapchainIndex);

        renderer.GetSwapchain().SubmitFrameCommand(CommandBufferStrongRef(commandBuffers, swapchainIndex));

        engine.Render();
        renderer.GetSwapchain().EndRendering();
    }
    renderer.DeviceWaitIdle();
    commandBuffers.reset();
    engine.GetWindowSystem().CloseWindow(window);
    return 0;
}
