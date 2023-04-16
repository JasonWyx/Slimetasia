#include "RenderFinalComposition.h"

#include "Logger.h"

RenderFinalComposition::RenderFinalComposition(const RenderContext& renderContext, const std::unique_ptr<SwapchainHandler>& swapchain)
    : RenderObject { renderContext }
    , m_SwapchainCache { swapchain }
{
    CreateDescriptors();
    CreateRenderPass();
    // CreateFramebuffers(); // Framebuffer is managed direct from swapchain
    CreatePipeline();
}

RenderFinalComposition::~RenderFinalComposition()
{
    RenderObject::DestroyDescriptors();
    RenderObject::DestroyRenderPass();
    // RenderObject::DestroyFramebuffers(); // Framebuffer is managed direct from swapchain
    RenderObject::DestroyPipeline();
}

RenderSyncObjects RenderFinalComposition::Render(const FrameInfo& frameInfo, const std::vector<vk::Semaphore>& waitSemaphores, const vk::Fence& signalFence)
{
    const vk::CommandBuffer& commandBuffer { m_CommandBuffers[frameInfo.frameIndex] };
    commandBuffer.reset();

    const vk::CommandBufferBeginInfo beginInfo {};
    commandBuffer.begin(beginInfo);

    const vk::ClearValue clearColor { .color { .float32 { { 1.0f, 1.0f, 1.0f, 1.0f } } } };
    const vk::RenderPassBeginInfo renderPassBeginInfo {
        .renderPass = m_RenderPass,
        .framebuffer = m_Framebuffers[frameInfo.swapchainIndex],
        .renderArea = { .extent { m_Context.m_Extent } },
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };

    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);

    const vk::Viewport viewport { .width = static_cast<float>(m_Context.m_Extent.width), .height = static_cast<float>(m_Context.m_Extent.height) };
    const vk::Rect2D scissor { .extent = m_Context.m_Extent };

    commandBuffer.setViewport(0, viewport);
    commandBuffer.setScissor(0, scissor);

    commandBuffer.draw(3, 1, 0, 0);
    commandBuffer.endRenderPass();
    commandBuffer.end();

    const vk::Semaphore& signalSemaphore = m_SignalSemaphores[frameInfo.frameIndex];
    const vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    const vk::SubmitInfo commandSubmitInfo {
        .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = &waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &signalSemaphore,
    };

    m_Context.m_Queues[QueueType::Graphics].submit(commandSubmitInfo, signalFence);

    return RenderSyncObjects { signalSemaphore };
}

void RenderFinalComposition::InitializeAfterSwapchain()
{
    m_Framebuffers = m_SwapchainCache->GetFramebuffers();
}

void RenderFinalComposition::CreateDescriptors()
{
    const std::vector<vk::DescriptorPoolSize> poolSizes = {
        vk::DescriptorPoolSize {
            .type = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = m_Context.m_FramesInFlight,
        },
    };
    const vk::DescriptorPoolCreateInfo poolCreateInfo {
        .maxSets = m_Context.m_FramesInFlight,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };
    m_DescriptorPool = m_Context.m_Device.createDescriptorPool(poolCreateInfo);

    // const vk::DescriptorSetAllocateInfo allocateInfo {
    //     .descriptorPool = m_DescriptorPool,
    //     .descriptorSetCount = 1,
    // };
    // m_DescriptorSets = m_Context.m_Device.allocateDescriptorSets(allocateInfo);
}

void RenderFinalComposition::CreateRenderPass()
{
    const vk::AttachmentDescription colorAttachment {
#ifdef EDITOR
        .format = m_SwapchainCache->GetSurfaceFormat().format,  // vk::Format::eR16G16B16A16Sfloat,
#else
        .format = m_SwapchainCache->GetSurfaceFormat().format,  // vk::Format::eR16G16B16A16Sfloat,
#endif  // EDITOR
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .initialLayout = vk::ImageLayout::eUndefined,
#ifdef EDITOR
        //.finalLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR,
#else
        .finalLayout = vk::ImageLayout::ePresentSrcKHR,
#endif  // EDITOR
    };

    const vk::AttachmentReference colorAttachmentRef {
        .attachment = 0,
        .layout = vk::ImageLayout::eColorAttachmentOptimal,
    };

    const vk::SubpassDescription subpass {
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
    };

    const vk::SubpassDependency subpassDep {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .srcAccessMask = vk::AccessFlagBits::eNone,
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
    };

    const vk::RenderPassCreateInfo createInfo {
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &subpassDep,
    };

    m_RenderPass = m_Context.m_Device.createRenderPass(createInfo);
}

void RenderFinalComposition::CreateFramebuffers()
{
    for (uint32_t i = 0; i < m_Context.m_FramesInFlight; ++i)
    {
        const vk::FramebufferCreateInfo createInfo {
            .attachmentCount = 1,
            // .pAttachments = // todo: assign allocated frame buffer image
            .width = m_Context.m_Extent.width,
            .height = m_Context.m_Extent.height,
            .layers = 1,
        };

        m_Framebuffers.push_back(m_Context.m_Device.createFramebuffer(createInfo));
    }
}

void RenderFinalComposition::CreatePipeline()
{
    // For uniform data and sorts
    const vk::PipelineLayoutCreateInfo layoutCreateInfo {};

    m_PipelineLayout = m_Context.m_Device.createPipelineLayout(layoutCreateInfo);

    const std::vector<char>& vertSpirv = ShaderHelper::CompileToSpirv("SimpleTriangle.vert.hlsl");
    const std::vector<char>& fragSpirv = ShaderHelper::CompileToSpirv("SimpleTriangle.frag.hlsl");

    const vk::ShaderModuleCreateInfo vertModuleCreateInfo {
        .codeSize = vertSpirv.size(),
        .pCode = reinterpret_cast<const uint32_t*>(vertSpirv.data()),
    };

    const vk::ShaderModuleCreateInfo fragModuleCreateInfo {
        .codeSize = fragSpirv.size(),
        .pCode = reinterpret_cast<const uint32_t*>(fragSpirv.data()),
    };

    const vk::ShaderModule vertShader { m_Context.m_Device.createShaderModule(vertModuleCreateInfo) };
    const vk::ShaderModule fragShader { m_Context.m_Device.createShaderModule(fragModuleCreateInfo) };

    const std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
        {
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = vertShader,
            .pName = "main",
        },
        {
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = fragShader,
            .pName = "main",
        },
    };

    const vk::PipelineVertexInputStateCreateInfo vertexInputState {
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr,
    };

    const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState {
        .topology = vk::PrimitiveTopology::eTriangleList,
    };

    const vk::Extent2D swapchainExtent = m_Context.m_Extent;

    const vk::Viewport viewport {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(swapchainExtent.width),
        .height = static_cast<float>(swapchainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    const vk::Rect2D scissor {
        .offset = { 0, 0 },
        .extent = swapchainExtent,
    };

    const vk::PipelineViewportStateCreateInfo viewportState {
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    const vk::PipelineRasterizationStateCreateInfo rasterizationState {
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eCounterClockwise,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
    };

    const vk::PipelineMultisampleStateCreateInfo multisampleState {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    const vk::PipelineDepthStencilStateCreateInfo depthStencilState {};  // todo: setup depth stencil

    const vk::PipelineColorBlendAttachmentState colorBlendAttachmentState {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = vk::BlendFactor::eOne,
        .dstColorBlendFactor = vk::BlendFactor::eZero,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
    };

    const vk::PipelineColorBlendStateCreateInfo colorBlendState {
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachmentState,
        .blendConstants = { { 0.0f, 0.0f, 0.0f, 0.0f } },
    };

    const std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

    const vk::PipelineDynamicStateCreateInfo dynamicState {
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };

    const vk::GraphicsPipelineCreateInfo pipelineCreateInfo {
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputState,
        .pInputAssemblyState = &inputAssemblyState,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizationState,
        .pMultisampleState = &multisampleState,
        .pDepthStencilState = &depthStencilState,
        .pColorBlendState = &colorBlendState,
        .pDynamicState = &dynamicState,
        .layout = m_PipelineLayout,
        .renderPass = m_RenderPass,
        .subpass = 0,
    };

    const vk::ResultValue<vk::Pipeline>& resultValue = m_Context.m_Device.createGraphicsPipeline({}, pipelineCreateInfo);
    ASSERT(resultValue.result == vk::Result::eSuccess);

    m_Pipeline = resultValue.value;

    m_Context.m_Device.destroyShaderModule(vertShader);
    m_Context.m_Device.destroyShaderModule(fragShader);
}
