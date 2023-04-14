#include "RenderFinalComposition.h"

#include "Logger.h"

RenderFinalComposition::RenderFinalComposition(const RenderContext& renderContext)
    : RenderObject { renderContext }
{
    CreateDescriptors();
    CreateRenderPass();
    CreateFramebuffers();
    CreatePipeline();
}

RenderFinalComposition::~RenderFinalComposition()
{
    DestroyDescriptors();
    DestroyRenderPass();
    DestroyFramebuffers();
    DestroyPipeline();
}

vk::Semaphore RenderFinalComposition::Render(const uint32_t currentFrame, const std::vector<vk::Semaphore>& waitSemaphores)
{
    const vk::CommandBuffer& commandBuffer = m_CommandBuffers[currentFrame];
    commandBuffer.reset();

    const vk::CommandBufferBeginInfo commandBufferBeginInfo {};
    commandBuffer.begin(commandBufferBeginInfo);

    const vk::ClearValue clearColor { .color { .float32 { { 1.0f, 1.0f, 1.0f, 1.0f } } } };
    const vk::RenderPassBeginInfo renderPassBeginInfo {
        .renderPass = m_RenderPass,
        .framebuffer = m_Framebuffers[currentFrame],
        .renderArea { .extent { m_Context.m_Extent } },
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };

    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);

    commandBuffer.endRenderPass();
    commandBuffer.end();

    const vk::Semaphore& signalSemaphore = m_SignalSemaphores[currentFrame];
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

    return signalSemaphore;
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
    const vk::DescriptorSetAllocateInfo allocateInfo {
        .descriptorPool = m_DescriptorPool,
    };

    m_DescriptorPool = m_Context.m_Device.createDescriptorPool(poolCreateInfo);
    m_DescriptorSets = m_Context.m_Device.allocateDescriptorSets(allocateInfo);
}

void RenderFinalComposition::DestroyDescriptors()
{
    m_Context.m_Device.freeDescriptorSets(m_DescriptorPool, m_DescriptorSets);
    m_Context.m_Device.destroyDescriptorPool(m_DescriptorPool);
}

void RenderFinalComposition::CreateRenderPass()
{
    const vk::AttachmentDescription colorAttachment {
        .format = vk::Format::eR16G16B16A16Sfloat,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .initialLayout = vk::ImageLayout::eUndefined,
#ifdef EDITOR
        .finalLayout = vk::ImageLayout::eColorAttachmentOptimal,
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

void RenderFinalComposition::DestroyRenderPass()
{
    m_Context.m_Device.destroyRenderPass(m_RenderPass);
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

void RenderFinalComposition::DestroyFramebuffers()
{
    std::ranges::for_each(m_Framebuffers, [this](const vk::Framebuffer& framebuffer) { m_Context.m_Device.destroyFramebuffer(framebuffer); });
}

void RenderFinalComposition::CreatePipeline()
{
    // For uniform data and sorts
    const vk::PipelineLayoutCreateInfo layoutCreateInfo {};

    m_PipelineLayout = m_Context.m_Device.createPipelineLayout(layoutCreateInfo);

    const std::vector<char>& vertSpirv = ShaderHelper::CompileToSpirv("SimpleTriangle.vert");
    const std::vector<char>& fragSpirv = ShaderHelper::CompileToSpirv("SimpleTriangle.frag");

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

void RenderFinalComposition::DestroyPipeline()
{
    m_Context.m_Device.destroyPipelineLayout(m_PipelineLayout);
    m_Context.m_Device.destroyPipeline(m_Pipeline);
}
