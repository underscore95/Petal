#include "VulkanGraphicsPipeline.h"

#include "RenderingDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanShader.h"
#include "Window/Window.h"

namespace Petal {
    VulkanGraphicsPipeline::VulkanGraphicsPipeline(
        GraphicsContext &renderer,
        const VulkanShader &shader,
        const std::shared_ptr<Logger> &logger,
        const PipelineSettings &settings,
        Result &resultOut
    ): m_renderer(renderer),
       m_shader(shader),
       m_logger(logger),
       m_settings(settings) {
        resultOut = CreatePipelineLayout();
        if (resultOut != Result::SUCCESS) return;

        resultOut = CreatePipeline();
        if (resultOut != Result::SUCCESS) return;

        m_logger->Verbose("Created graphics pipeline");
    }

    VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
        vkDestroyPipeline(
            m_renderer.GetDevice()->GetDevice(),
            m_pipeline,
            nullptr
        );

        vkDestroyPipelineLayout(
            m_renderer.GetDevice()->GetDevice(),
            m_pipelineLayout,
            nullptr
        );
    }

    VkPipeline VulkanGraphicsPipeline::GetHandle() const {
        return m_pipeline;
    }

    VkPipelineLayout VulkanGraphicsPipeline::GetLayout() const {
        return m_pipelineLayout;
    }

    Result VulkanGraphicsPipeline::CreatePipelineLayout() {
        VkPushConstantRange pushConstantRange = {
            .stageFlags = VK_SHADER_STAGE_ALL,
            .offset = 0,
            .size = m_renderer.GetGraphicsSettings().PushConstantSize
        };

        const std::vector<VkDescriptorSetLayout> &layouts = m_shader.GetDescriptorSetLayouts();
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<glm::u32>(layouts.size()),
            .pSetLayouts = layouts.data(),
            .pushConstantRangeCount = m_renderer.GetGraphicsSettings().PushConstantSize > 0 ? 1u : 0u, // no need for a range if we don't have any
            .pPushConstantRanges = &pushConstantRange
        };

        VkResult res = vkCreatePipelineLayout(m_renderer.GetDevice()->GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
        PETAL_CHECK_COND(res != VK_SUCCESS, Result::VULKAN_PIPELINE_CREATION_FAILED, m_logger, "Failed to create pipeline layout: {}", res);
        return Result::SUCCESS;
    }

    Result VulkanGraphicsPipeline::CreatePipeline() {
        glm::uvec2 windowSize = m_renderer.GetWindow().GetDimensions();

        // std::vector<VkViewport> viewports = {
        //     VkViewport{
        //         .x = 0,
        //         .y = 0,
        //         .width = static_cast<float>(windowSize.x),
        //         .height = static_cast<float>(windowSize.y),
        //         .minDepth = 0,
        //         .maxDepth = 1
        //     }
        // };
        //
        // std::vector<VkRect2D> scissors = {
        //     VkRect2D{
        //         .offset = {0, 0}, .extent = {windowSize.x, windowSize.y}
        //     }
        // };
        //
        // VkPipelineViewportStateCreateInfo viewportState = {
        //     .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        //     .pNext = nullptr,
        //     .flags = 0,
        //     .viewportCount = static_cast<glm::u32>(viewports.size()),
        //     .pViewports = viewports.data(),
        //     .scissorCount = static_cast<glm::u32>(scissors.size()),
        //     .pScissors = scissors.data()
        // };

        // VkPipelineColorBlendStateCreateInfo colorBlending = {};
        // colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        // colorBlending.pNext = nullptr;
        //
        // colorBlending.logicOpEnable = VK_FALSE;
        // colorBlending.logicOp = VK_LOGIC_OP_COPY;
        // colorBlending.attachmentCount = 1;
        // colorBlending.pAttachments = &_colorBlendAttachment;
        //
        // VkPipelineVertexInputStateCreateInfo _vertexInputInfo = {
        //     .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        //     .pNext = nullptr,
        //     .flags = 0,
        //     .vertexBindingDescriptionCount = 0,
        //     .pVertexBindingDescriptions = nullptr,
        //     .vertexAttributeDescriptionCount = 0,
        //     .pVertexAttributeDescriptions = nullptr
        // };

        //         VkPipelineInputAssemblyStateCreateInfo inputAssembly={
        // .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO ,
        // .pNext =nullptr ,
        // .flags = ,
        // .topology = ,
        // .primitiveRestartEnable =
        //         };

        // Rendering info
        Optional<VkFormat> depthFormatOptional = m_renderer.GetDevice()->FindDepthFormat();
        VkFormat swapchainFormat = m_renderer.GetSwapchain().GetSurfaceFormat().surfaceFormat.format;

        VkPipelineRenderingCreateInfo renderingInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
            .pNext = nullptr,
            .viewMask = 0,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &swapchainFormat,
            .depthAttachmentFormat = *depthFormatOptional.Value(),
            .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
        };

        // Shader stages
        std::vector<VkPipelineShaderStageCreateInfo> stages;
        for (const VulkanShader::Stage &stage : m_shader.GetShaderStages()) {
            stages.push_back(VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stage = static_cast<VkShaderStageFlagBits>(ShaderTypes::GetData(stage.Type).VulkanShaderStage),
                .module = stage.ShaderModule,
                .pName = stage.EntryPointFunctionName.c_str(),
                .pSpecializationInfo = nullptr
            });
        }

        // Vertex input state
        VkPipelineVertexInputStateCreateInfo vertexInputState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
        };

        // Input assembly state
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
        };

        // Viewport state
        // viewports and scissors are allowed to be null as long as their state is dynamic
        VkPipelineViewportStateCreateInfo viewportState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = nullptr,
            .scissorCount = 1,
            .pScissors = nullptr
        };

        // Rasterization state
        VkPipelineRasterizationStateCreateInfo rasterizationState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .polygonMode = m_settings.RenderWireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL,
            .cullMode = static_cast<VkCullModeFlags>(m_settings.CullBackFaces ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE),
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .lineWidth = 1.0f
        };

        // Multisampling state
        VkPipelineMultisampleStateCreateInfo multisamplingState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f
        };

        // Depth stencil state
        VkPipelineDepthStencilStateCreateInfo depthStencilState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = false, // todo
            .depthWriteEnable = false, // todo
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = 0.0f,
            .maxDepthBounds = 1.0f
        };

        // Colour blend state
        VkPipelineColorBlendAttachmentState blendAttachState = {
            .blendEnable = VK_FALSE, // todo
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                              VK_COLOR_COMPONENT_A_BIT
        };

        VkPipelineColorBlendStateCreateInfo colorBlendState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &blendAttachState
        };

        // Dynamic state
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .dynamicStateCount = static_cast<glm::u32>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
        };

        VkGraphicsPipelineCreateInfo pipelineInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &renderingInfo,
            .flags = 0,
            .stageCount = static_cast<glm::u32>(stages.size()),
            .pStages = stages.data(),
            .pVertexInputState = &vertexInputState,
            .pInputAssemblyState = &inputAssemblyState,
            .pTessellationState = nullptr,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizationState,
            .pMultisampleState = &multisamplingState,
            .pDepthStencilState = &depthStencilState,
            .pColorBlendState = &colorBlendState,
            .pDynamicState = &dynamicState,
            .layout = m_pipelineLayout,
            .renderPass = nullptr,
            .subpass = 0,
            .basePipelineHandle = nullptr,
            .basePipelineIndex = -1
        };

        VkResult result = vkCreateGraphicsPipelines(
            m_renderer.GetDevice()->GetDevice(),
            nullptr, // todo
            1,
            &pipelineInfo,
            nullptr,
            &m_pipeline
        );

        PETAL_CHECK_COND(
            result != VK_SUCCESS,
            Result::VULKAN_PIPELINE_CREATION_FAILED,
            m_logger,
            "Failed to create graphics pipeline: {}", result
        );

        return Result::SUCCESS;
    }
} // Petal
