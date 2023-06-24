#include "RenderImGui.h"

#include "External Libraries/imgui/backends/imgui_impl_vulkan.h"
#include "External Libraries/imgui/backends/imgui_impl_win32.h"
#include "RendererVk.h"

RenderImGui::RenderImGui(const RenderContext& renderContext, const vk::Instance instance, const vk::PhysicalDevice physicalDevice)
    : RenderObject { renderContext }
{
    CreateDescriptors();
    // CreateFramebuffers(); // Framebuffer is managed direct from swapchain
    CreateRenderPass();

    ImGui_ImplVulkan_InitInfo imguiInitInfo {
        .Instance = instance,
        .PhysicalDevice = physicalDevice,
        .Device = renderContext.m_Device,
        .QueueFamily = renderContext.m_QueueIndices[QueueType::Graphics],
        .Queue = renderContext.m_Queues[QueueType::Graphics],
        .DescriptorPool = m_DescriptorPool,
        .Subpass = 0,
        .MinImageCount = renderContext.m_FramesInFlight,
        .ImageCount = renderContext.m_SwapchainCount,
    };

    ImGui_ImplVulkan_Init(&imguiInitInfo, m_RenderPass);

    const vk::CommandBufferBeginInfo beginInfo { .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
    m_CommandBuffers[0].begin(beginInfo);
    ImGui_ImplVulkan_CreateFontsTexture(m_CommandBuffers[0]);
    m_CommandBuffers[0].end();

    const vk::SubmitInfo submitInfo { .commandBufferCount = 1, .pCommandBuffers = &(m_CommandBuffers[0]) };
    renderContext.m_Queues[QueueType::Transfer].submit(submitInfo);
    renderContext.m_Device.waitIdle();  // todo: maybe wait on fence?

    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

RenderImGui::~RenderImGui()
{
    ImGui_ImplVulkan_Shutdown();

    DestroyRenderPass();
    // DestroyFramebuffers(); // Framebuffer is managed direct from swapchain
    DestroyDescriptors();
}

RenderOutputs RenderImGui::Render(const FrameInfo& frameInfo, const std::vector<vk::Semaphore>& waitSemaphores, const vk::Fence& signalFence)
{
    const vk::CommandBuffer commandBuffer { m_CommandBuffers[frameInfo.m_FrameIndex] };
    commandBuffer.reset();

    const vk::CommandBufferBeginInfo beginInfo {};
    commandBuffer.begin(beginInfo);

    const vk::ClearValue clearColor { vk::ClearColorValue{ 0.0f, 0.0f, 0.0f, 1.0f } };
    const vk::RenderPassBeginInfo renderPassBeginInfo {
        .renderPass = m_RenderPass,
        .framebuffer = m_Framebuffers[frameInfo.m_SwapchainIndex],
        .renderArea = { .extent = m_Context.m_WindowExtent, },
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };

    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    ImDrawData* drawData = ImGui::GetDrawData();
    ASSERT(drawData != nullptr);

    ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);

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

void RenderImGui::OnSwapchainFramebuffersChanged()
{
    CreateFramebuffers();
}

void RenderImGui::CreateDescriptors()
{
    std::vector<vk::DescriptorPoolSize> poolSizes { { vk::DescriptorType::eSampler, 1000 },
                                                    { vk::DescriptorType::eCombinedImageSampler, 1000 },
                                                    { vk::DescriptorType::eSampledImage, 1000 },
                                                    { vk::DescriptorType::eStorageImage, 1000 },
                                                    { vk::DescriptorType::eUniformTexelBuffer, 1000 },
                                                    { vk::DescriptorType::eStorageTexelBuffer, 1000 },
                                                    { vk::DescriptorType::eUniformBuffer, 1000 },
                                                    { vk::DescriptorType::eStorageBuffer, 1000 },
                                                    { vk::DescriptorType::eUniformBufferDynamic, 1000 },
                                                    { vk::DescriptorType::eStorageBufferDynamic, 1000 },
                                                    { vk::DescriptorType::eInputAttachment, 1000 } };

    vk::DescriptorPoolCreateInfo createInfo {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = static_cast<uint32_t>(1000 * poolSizes.size()),
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };

    m_DescriptorPool = m_Context.m_Device.createDescriptorPool(createInfo);
}

void RenderImGui::CreateRenderPass()
{
    const vk::Format targetFormat = g_Renderer->GetSwapchainHandler()->GetSurfaceFormat().format;

    const vk::AttachmentDescription colorAttachment {
        .format = targetFormat,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR,
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

    // Initial dependency to transition
    const vk::SubpassDependency dependency {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .srcAccessMask = vk::AccessFlagBits::eNone,
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead,
    };

    const vk::RenderPassCreateInfo createInfo {
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    m_RenderPass = m_Context.m_Device.createRenderPass(createInfo);
}

void RenderImGui::CreateFramebuffers()
{
    m_Framebuffers = g_Renderer->GetSwapchainHandler()->GetFramebuffers();
}

void RenderImGui::CreatePipeline() {}