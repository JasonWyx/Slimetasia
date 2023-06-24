#include "RenderFinal.h"

#include "External Libraries/imgui/backends/imgui_impl_vulkan.h"
#include "Logger.h"
#include "RendererVk.h"

RenderFinal::RenderFinal(const RenderContext& renderContext)
    : RenderObject { renderContext }
{
    CreateDescriptors();
    CreateRenderPass();
    CreateFramebuffers();
    CreatePipeline();
}

RenderFinal::~RenderFinal()
{
    DestroyDescriptors();
    DestroyRenderPass();
    DestroyFramebuffers();
    DestroyPipeline();
}

RenderOutputs RenderFinal::Render(const FrameInfo& frameInfo, const std::vector<vk::Semaphore>& waitSemaphores, const vk::Fence& signalFence)
{
    const vk::CommandBuffer& commandBuffer { m_CommandBuffers[frameInfo.m_FrameIndex] };
    commandBuffer.reset();

    const vk::CommandBufferBeginInfo beginInfo {};
    commandBuffer.begin(beginInfo);

    const vk::ClearValue clearColor { vk::ClearColorValue{ 1.0f, 1.0f, 1.0f, 1.0f } };
    const vk::RenderPassBeginInfo renderPassBeginInfo {
        .renderPass = m_RenderPass,
        .framebuffer = m_Framebuffers[frameInfo.m_SwapchainIndex],
        .renderArea = { .extent { m_Context.m_WindowExtent } },
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };

    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);

    const vk::Viewport viewport { .width = static_cast<float>(m_Context.m_WindowExtent.width), .height = static_cast<float>(m_Context.m_WindowExtent.height) };
    const vk::Rect2D scissor { .extent = m_Context.m_WindowExtent };

    commandBuffer.setViewport(0, viewport);
    commandBuffer.setScissor(0, scissor);

    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRenderPass();
    commandBuffer.end();

    const vk::Semaphore& signalSemaphore = m_SignalSemaphores[frameInfo.m_FrameIndex];
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

    return RenderOutputs { signalSemaphore };
}

void RenderFinal::SetWindowExtent(const vk::Extent2D& extent)
{
    RenderObject::SetWindowExtent(extent);
}

void RenderFinal::SetRenderExtent(const vk::Extent2D& extent)
{
    if (extent != m_Context.m_RenderExtent)
    {
    }

    RenderObject::SetRenderExtent(extent);
}

void RenderFinal::SetIsRenderToTarget(const bool isRenderToTarget)
{
    if (isRenderToTarget != m_Context.m_IsRenderToTarget)
    {
        DestroyFramebuffers();
        DestroyRenderPass();

        m_Context.m_IsRenderToTarget = isRenderToTarget;

        CreateFramebuffers();
        CreateRenderPass();
    }
}

void RenderFinal::OnSwapchainFramebuffersChanged()
{
    m_Framebuffers = g_Renderer->GetSwapchainHandler()->GetFramebuffers();
}

VkDescriptorSet RenderFinal::GetRenderAttachment(const uint32_t frameIndex) const
{
    return m_RenderAttachments[frameIndex];
}

void RenderFinal::CreateDescriptors()
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
}

void RenderFinal::CreateRenderPass()
{
    vk::Format targetFormat {};
    vk::ImageLayout targetLayout {};

    if (m_Context.m_IsRenderToTarget)
    {
        targetFormat = g_Renderer->GetSwapchainHandler()->GetSurfaceFormat().format;
        targetLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    }
    else
    {
        targetFormat = g_Renderer->GetSwapchainHandler()->GetSurfaceFormat().format;
        targetLayout = vk::ImageLayout::ePresentSrcKHR;
    }

    const vk::AttachmentDescription colorAttachment {
        .format = targetFormat,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = targetLayout,
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

void RenderFinal::CreateFramebuffers()
{
    if (m_Context.m_IsRenderToTarget)
    {
        const vk::Extent3D extent { m_Context.m_RenderExtent.width, m_Context.m_RenderExtent.height, 1 };
        const vk::Format renderFormat = g_Renderer->GetSwapchainHandler()->GetSurfaceFormat().format;
        const vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

        for (uint32_t i = 0; i < m_Context.m_FramesInFlight; ++i)
        {
            ImageObject* renderTarget = ImageObject::CreateImage(renderFormat, extent, usageFlags, false, nullptr);
            renderTarget->GenerateView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
            renderTarget->GenerateSampler(vk::Filter::eNearest, vk::SamplerAddressMode::eClampToBorder);

            const vk::ImageView renderView = renderTarget->GetView();

            m_RenderTargets.push_back(renderTarget);
            m_RenderAttachments.push_back(ImGui_ImplVulkan_AddTexture(renderTarget->GetSampler(), renderTarget->GetView(), (VkImageLayout)vk::ImageLayout::eShaderReadOnlyOptimal));

            const vk::FramebufferCreateInfo createInfo {
                .renderPass = m_RenderPass,
                .attachmentCount = 1,
                .pAttachments = &renderView,
                .width = extent.width,
                .height = extent.height,
                .layers = 1,
            };

            m_Framebuffers.push_back(m_Context.m_Device.createFramebuffer(createInfo));
        }
    }
}

void RenderFinal::DestroyFramebuffers()
{
    // If not render to target, framebuffer is managed by swapchain
    if (m_Context.m_IsRenderToTarget)
    {
        for (const VkDescriptorSet& renderAttachment : m_RenderAttachments)
        {
            ImGui_ImplVulkan_RemoveTexture(renderAttachment);
        }
        m_RenderAttachments.clear();

        for (const ImageObject* renderTarget : m_RenderTargets)
        {
            delete renderTarget;
        }
        m_RenderTargets.clear();

        RenderObject::DestroyFramebuffers();
    }
}

void RenderFinal::CreatePipeline()
{
    // For uniform data and sorts
    const vk::PipelineLayoutCreateInfo layoutCreateInfo {};

    m_PipelineLayout = m_Context.m_Device.createPipelineLayout(layoutCreateInfo);

    ShaderModuleObject vertModule { "SimpleTriangle.vert.hlsl", m_Context.m_Device };
    ShaderModuleObject fragModule { "SimpleTriangle.frag.hlsl", m_Context.m_Device };

    const std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
        {
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = vertModule.GetModule(),
            .pName = "main",
        },
        {
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = fragModule.GetModule(),
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

    const vk::Extent2D swapchainExtent = m_Context.m_WindowExtent;

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
}
