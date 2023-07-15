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

    const vk::CommandBufferBeginInfo beginInfo { vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
    m_CommandBuffers[0].begin(beginInfo);
    ImGui_ImplVulkan_CreateFontsTexture(m_CommandBuffers[0]);
    m_CommandBuffers[0].end();

    const vk::SubmitInfo submitInfo { {}, {}, m_CommandBuffers[0] };
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

    const vk::ClearValue clearColor { vk::ClearColorValue { 0.0f, 0.0f, 0.0f, 1.0f } };
    const vk::RenderPassBeginInfo renderPassBeginInfo { m_RenderPass, m_Framebuffers[frameInfo.m_SwapchainIndex], vk::Rect2D { {}, m_Context.m_WindowExtent }, clearColor };

    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    ImDrawData* drawData = ImGui::GetDrawData();
    ASSERT(drawData != nullptr);

    ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);

    commandBuffer.endRenderPass();
    commandBuffer.end();

    const vk::Semaphore& signalSemaphore = m_SignalSemaphores[frameInfo.m_FrameIndex];
    const std::vector<vk::PipelineStageFlags> waitStages { waitSemaphores.size(), vk::PipelineStageFlagBits::eColorAttachmentOutput };
    const vk::SubmitInfo commandSubmitInfo { waitSemaphores, waitStages, commandBuffer, signalSemaphore };

    m_Context.m_Queues[QueueType::Graphics].submit(commandSubmitInfo, signalFence);

    return RenderOutputs { signalSemaphore };
}

void RenderImGui::OnSwapchainFramebuffersChanged()
{
    CreateFramebuffers();
}

void RenderImGui::CreateDescriptors()
{
    const std::vector<vk::DescriptorPoolSize> poolSizes { { vk::DescriptorType::eSampler, 1000 },
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

    const vk::DescriptorPoolCreateInfo createInfo { vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, m_Context.m_FramesInFlight * 1000, poolSizes };

    m_DescriptorPool = m_Context.m_Device.createDescriptorPool(createInfo);
}

void RenderImGui::CreateRenderPass()
{
    const vk::Format targetFormat = g_Renderer->GetSwapchainHandler()->GetSurfaceFormat().format;

    const vk::AttachmentDescription colorAttachment { {},
                                                      targetFormat,
                                                      vk::SampleCountFlagBits::e1,
                                                      vk::AttachmentLoadOp::eClear,
                                                      vk::AttachmentStoreOp::eStore,
                                                      vk::AttachmentLoadOp::eClear,
                                                      vk::AttachmentStoreOp::eStore,
                                                      vk::ImageLayout::eUndefined,
                                                      vk::ImageLayout::ePresentSrcKHR };

    const vk::AttachmentReference colorAttachmentRef { 0, vk::ImageLayout::eColorAttachmentOptimal };

    const vk::SubpassDescription subpass { {}, vk::PipelineBindPoint::eGraphics, {}, colorAttachmentRef };

    // Initial dependency to transition
    const vk::SubpassDependency dependency { VK_SUBPASS_EXTERNAL,
                                             0,
                                             vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                             vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                             vk::AccessFlagBits::eNone,
                                             vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead };

    const vk::RenderPassCreateInfo createInfo { {}, colorAttachment, subpass, dependency };

    m_RenderPass = m_Context.m_Device.createRenderPass(createInfo);
}

void RenderImGui::CreateFramebuffers()
{
    m_Framebuffers = g_Renderer->GetSwapchainHandler()->GetFramebuffers();
}

void RenderImGui::CreatePipeline() {}