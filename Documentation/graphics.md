# Graphics

The Graphics system provides a low level abstraction over Vulkan, this should only be used in advanced projects and most projects should use the Rendering system.

## Graphics System

This is the graphics entry point and handles graphics API initialization and shutdown.

## Graphics Context

A graphics context handles graphics API intialization and shutdown for objects tied to a specific window or device (surface, logical device, swapchain, buffers, etc).

### Shaders

To compile a shader, use the following function:

```AllocatedOptional<VulkanShader> CompileShader(const ShaderAsset &asset)```

This will compile the Slang shader source files into SPIRV and perform reflection to automatically create all required descriptors.

The `VulkanShader` class provides these functions:

Bind a buffer to the shader by using the name of the buffer in the shader:

```Result BindBuffer(const std::string &name, const GPUBuffer &buffer)```

Bind all resources to the command buffer, this must be called once before the shader is used
in a command buffer.

```void BindResources(VkCommandBuffer commandBuffer)```

### Command Buffers

Create a command buffer.
It is recommended to move the command buffer into a shared ptr after creation so it can be converted into a CommandBufferRef
```
  AllocatedOptional<CommandBuffer> CreateCommandBuffer(
    const VulkanQueue &queueFamily,
    VkCommandBufferLevel level
)
```

Create N command buffers.
It is recommended to move the command buffers into a shared ptr after creation so it can be converted into a CommandBufferRef
        
```
AllocatedOptional<CommandBufferVector> CreateCommandBuffers(
    const VulkanQueue &queueFamily,
    VkCommandBufferLevel level,
    glm::u32 count
)
```

Create N command buffers.
It is recommended to move the command buffers into a shared ptr after creation so it can be converted into a CommandBufferRef
  The lambda is executed once per command buffer to record, you do not need to begin/end inside this lambda
    
```
AllocatedOptional<CommandBufferVector> CreateCommandBuffersWithContents(
    const VulkanQueue &queueFamily,
    VkCommandBufferLevel level,
    glm::u32 count,
    VkCommandBufferUsageFlags usageFlags,
    const std::function<void(VkCommandBuffer, glm::u32)> &lambda
)
```

In Petal, it is common for a `CommandBufferVector` to have one command buffer per swapchain image (e.g. 3 for triple buffering) and some frame/graphical functions may take in a` CommandBufferVector`.

Both `CommandBufferVector` and `CommandBuffer` have `GetHandle()` functions to retrieve the Vulkan command buffer handle.

`CommandBufferRef` is a weak pointer to a Vulkan command buffer, this could be contained in a `CommandBuffer` or a `CommandBufferVector`.

`CommandBufferStrongRef` is a shared pointer to a Vulkan command buffer, this could be contained in a `CommandBuffer` or a `CommandBufferVector`.

### Buffers

These are handled by the buffer subsystem, accessible via:

```
GPUBufferSubsystem &GetBufferSubsystem()
```

This class provides functions to create a `GPUBuffer`, or to write data into a `GPUBuffer`.

In the future, `GPUBuffer` will be an abstraction over part of a graphics API buffer, and multiple `GPUBuffer` instances may be backed by different parts of the same graphics API buffer.

#### Writing Data

This function will write to the GPU buffer by writing to an internal transfer buffer and copying from the transfer buffer into the actual buffer. This copying process is split into blocks if the transfer buffer is not big enough. This function blocks until copying is complete.

```
Result Write(
    const GPUBuffer &buffer,
    const void *data,
    glm::u32 size,
    glm::u32 bufferOffset = 0
)
```

### Swapchain

To render to the screen, the `VulkanSwapchain` should be used, accessible from the `GraphicsContext`.

The number of swapchain images can be set when passing `GraphicsSettings` into the `CreateGraphicsContext` (note: not currently possible) and retrieved via the swapchain:

```
glm::u32 NumSwapchainImages()
```

A `CommandBufferVector` with one command buffer per swapchain image should be created and the following should be recorded into it:

This sets up the command buffer so that the graphics API is ready to render.
```
void CmdBeginRendering(const CommandBufferVector &commandBuffers)
```

```
shaderOpt->BindResources(const CommandBufferVector &commandBuffers)
```