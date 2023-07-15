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

    const vk::ClearValue clearColor { vk::ClearColorValue {} };
    const vk::RenderPassBeginInfo renderPassBeginInfo { m_RenderPass, m_Framebuffers[frameInfo.m_SwapchainIndex], vk::Rect2D { {}, m_Context.m_WindowExtent }, clearColor };

    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);

    const vk::Viewport viewport { 0.0f, 0.0f, static_cast<float>(m_Context.m_WindowExtent.width), static_cast<float>(m_Context.m_WindowExtent.height) };
    const vk::Rect2D scissor { {}, m_Context.m_WindowExtent };

    commandBuffer.setViewport(0, viewport);
    commandBuffer.setScissor(0, scissor);

    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRenderPass();
    commandBuffer.end();

    const vk::Semaphore& signalSemaphore = m_SignalSemaphores[frameInfo.m_FrameIndex];
    const std::vector<vk::PipelineStageFlags> waitStages { waitSemaphores.size(), vk::PipelineStageFlagBits::eColorAttachmentOutput };
    const vk::SubmitInfo commandSubmitInfo { waitSemaphores, waitStages, commandBuffer, signalSemaphore };

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
        vk::DescriptorPoolSize { vk::DescriptorType::eCombinedImageSampler, m_Context.m_FramesInFlight },
    };
    const vk::DescriptorPoolCreateInfo poolCreateInfo { {}, m_Context.m_FramesInFlight, poolSizes };

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

    const vk::AttachmentDescription colorAttachment { {}, targetFormat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, targetLayout };

    const vk::AttachmentReference colorAttachmentRef { 0, vk::ImageLayout::eColorAttachmentOptimal };
    const vk::SubpassDescription subpass { {}, vk::PipelineBindPoint::eGraphics, {}, colorAttachmentRef, {}, {}, {} };
    const vk::SubpassDependency subpassDep { VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eNone,
        vk::AccessFlagBits::eColorAttachmentWrite };

    const vk::RenderPassCreateInfo createInfo { {}, colorAttachment, subpass, subpassDep };

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

            const vk::FramebufferCreateInfo createInfo { {}, m_RenderPass, renderView, extent.width, extent.height, 1 };

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
        vk::PipelineShaderStageCreateInfo { {}, vk::ShaderStageFlagBits::eVertex, vertModule.GetModule(), "main" },
        vk::PipelineShaderStageCreateInfo { {}, vk::ShaderStageFlagBits::eFragment, fragModule.GetModule(), "main" },
    };

    const vk::PipelineVertexInputStateCreateInfo vertexInputState {};
    const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState { {}, vk::PrimitiveTopology::eTriangleList };

    const vk::Extent2D swapchainExtent = m_Context.m_WindowExtent;
    const vk::Viewport viewport { 0.0f, 0.0f, static_cast<float>(swapchainExtent.width), static_cast<float>(swapchainExtent.height), 0.0f, 1.0f };
    const vk::Rect2D scissor { {}, swapchainExtent };

    const vk::PipelineViewportStateCreateInfo viewportState { {}, viewport, scissor };
    const vk::PipelineRasterizationStateCreateInfo rasterizationState { {}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, VK_FALSE, 0.f,
        0.f, 0.f, 1.0f };
    const vk::PipelineMultisampleStateCreateInfo multisampleState { {}, vk::SampleCountFlagBits::e1, VK_FALSE, 1.0f };
    const vk::PipelineDepthStencilStateCreateInfo depthStencilState {};  // todo: setup depth stencil
    const vk::PipelineColorBlendAttachmentState colorBlendAttachmentState { VK_FALSE, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eOne, vk::BlendFactor::eZero,
        vk::BlendOp::eAdd, vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags };
    const vk::PipelineColorBlendStateCreateInfo colorBlendState { {}, VK_FALSE, vk::LogicOp::eClear, colorBlendAttachmentState };

    const std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    const vk::PipelineDynamicStateCreateInfo dynamicState { {}, dynamicStates };

    const vk::GraphicsPipelineCreateInfo pipelineCreateInfo { {}, shaderStages, &vertexInputState, &inputAssemblyState, nullptr, &viewportState, &rasterizationState, &multisampleState,
        &depthStencilState, &colorBlendState, &dynamicState, m_PipelineLayout, m_RenderPass };

    const vk::ResultValue<vk::Pipeline>& resultValue = m_Context.m_Device.createGraphicsPipeline({}, pipelineCreateInfo);
    ASSERT(resultValue.result == vk::Result::eSuccess);

    m_Pipeline = resultValue.value;
}
