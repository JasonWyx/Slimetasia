#include "RendererVk.h"

#include <vulkan/vulkan_win32.h>

#include <cstring>
#include <iostream>
#include <optional>
#include <ranges>
#include <set>

import ShaderHelper;

static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

RendererVk::RendererVk(HINSTANCE appInstance, HWND appWindow, const uint32_t windowWidth, const uint32_t windowHeight)
    : m_AppWindow { appWindow }
    , m_CurrentFrame { 0 }
#ifdef EDITOR
    , m_IsRenderToTarget { true }
#endif
{
    CreateInstance();
    CreateSurface(appInstance, appWindow);
    ChoosePhysicalDevice();
    CreateDevice();
    CreateSwapchain(appWindow);
    CreateHandlers();
    CreateCommandPool();
    CreateSyncObjects();
}

RendererVk::~RendererVk()
{
    m_Device.waitIdle();

    // Release renderers
    m_RenderFinal.reset();
    m_RenderGBuffer.reset();
#ifdef EDITOR
    m_RenderImGui.reset();
#endif  // EDITOR

    // Release device objects
    std::ranges::for_each(m_ImageAvailableSemaphores, [this](const vk::Semaphore& semaphore) { m_Device.destroySemaphore(semaphore); });
    std::ranges::for_each(m_InFlightFences, [this](const vk::Fence& fence) { m_Device.destroyFence(fence); });
    m_Device.destroyCommandPool(m_OneShotCommandPool);

    // Release handlers
    m_MemoryHandler.reset();
    m_SwapchainHandler.reset();

    m_Instance.destroySurfaceKHR(m_Surface);

    // Release core objects
    m_Device.destroy();
    m_Instance.destroy();
}

void RendererVk::PostInitialize()
{
    CreateRenderers();
    CreateFramebuffers();
}

void RendererVk::Update(const float deltaTime)
{
    // Check frame in flight
    const vk::Fence& inFlightFence = m_InFlightFences[m_CurrentFrame];
    const vk::Result waitResult = m_Device.waitForFences(inFlightFence, VK_TRUE, UINT64_MAX);
    m_Device.resetFences(inFlightFence);

    const vk::ResultValue<uint32_t>& swapchainAcquireResult = m_SwapchainHandler->AcquireNextImageIndex(m_ImageAvailableSemaphores[m_CurrentFrame]);

    if (swapchainAcquireResult.result == vk::Result::eErrorOutOfDateKHR)
    {
        OnWindowResized();
        return;
    }

    const uint32_t imageIndex = swapchainAcquireResult.value;
    const std::vector<vk::Framebuffer>& framebuffers { m_SwapchainHandler->GetFramebuffers() };
    std::vector<vk::Semaphore> waitRenderSemaphores { m_ImageAvailableSemaphores[m_CurrentFrame] };
    std::vector<vk::Semaphore> waitPresentSemaphores {};

    const FrameInfo& frameInfo { m_CurrentFrame, imageIndex };

#ifdef EDITOR
    const RenderOutputs& renderFinalOutputs = m_RenderFinal->Render(frameInfo, {}, {});
    waitRenderSemaphores.push_back(renderFinalOutputs.m_SignaledSemaphore);

    const RenderOutputs& renderImGuiOutputs = m_RenderImGui->Render(frameInfo, waitRenderSemaphores, inFlightFence);
    waitPresentSemaphores.push_back(renderImGuiOutputs.m_SignaledSemaphore);
#else
    const RenderOutputs& renderFinalOutputs = m_RenderFinal->Render(frameInfo, waitRenderSemaphores, inFlightFence);
    waitPresentSemaphores.push_back(renderFinalOutputs.m_SignaledSemaphore);
#endif  // EDITOR

    const vk::SwapchainKHR swapchain = m_SwapchainHandler->GetSwapchain();
    const vk::PresentInfoKHR presentInfo { waitPresentSemaphores, swapchain, imageIndex };
    const vk::Result presentResult = m_Queues[QueueType::Present].presentKHR(presentInfo);

    if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR)
    {
        OnWindowResized();
    }

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RendererVk::OnWindowResized()
{
    // Wait idle is called internally
    m_SwapchainHandler->RecreateSwapchain(m_PhysicalDevice, m_Device, m_Surface, m_AppWindow);

    CreateFramebuffers();

    const vk::Extent2D& extent = m_SwapchainHandler->GetExtent();

    m_RenderGBuffer->SetWindowExtent(extent);
    m_RenderFinal->SetWindowExtent(extent);
#ifdef EDITOR
    m_RenderImGui->SetWindowExtent(extent);
#endif  // EDITOR
}

void RendererVk::CreateInstance()
{
    // Instance layers
    const std::vector<const char*> layerNames = {
#if defined(_DEBUG)
        "VK_LAYER_KHRONOS_validation"
#endif  // #if defined(_DEBUG)
        "VK_LAYER_KHRONOS_validation"
    };

    // Instance extensions
    const std::vector<const char*> extensionNames = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#ifdef EDITOR
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif  // EDITOR
    };

    // Check instance layers exists
    const std::vector<vk::LayerProperties> layerPropertiesList = vk::enumerateInstanceLayerProperties();
    for (const char* layerName : layerNames)
    {
        bool matchFound = false;

        for (const vk::LayerProperties& layerProperties : layerPropertiesList)
        {
            if (std::strcmp(layerName, layerProperties.layerName.data()))
            {
                matchFound = true;
            }
        }

        if (!matchFound)
        {
            std::cerr << "Required vulkan instance layer missing: " << layerName << std::endl;
        }
    }

    const vk::ApplicationInfo applicationInfo { "Slimetasia", 1, "Slimetasia Engine", 1, VK_API_VERSION_1_3 };
    const vk::InstanceCreateInfo createInfo { {}, &applicationInfo, layerNames, extensionNames };

    m_Instance = vk::createInstance(createInfo);
}

void RendererVk::CreateSurface(const HINSTANCE hInstance, const HWND hWindow)
{
    const vk::Win32SurfaceCreateInfoKHR createInfo { {}, hInstance, hWindow };

    m_Surface = m_Instance.createWin32SurfaceKHR(createInfo);
}

void RendererVk::ChoosePhysicalDevice()
{
    const std::vector<vk::PhysicalDevice> physicalDevices = m_Instance.enumeratePhysicalDevices();

    // Assuming only 1 physical device for now.
    // TODO: Implement choosing and scoring of devices

    m_PhysicalDevice = physicalDevices.front();
}

void RendererVk::CreateDevice()
{
    ASSERT(m_PhysicalDevice);
    ASSERT(m_Surface);

    // Getting queue indices
    const std::vector<vk::QueueFamilyProperties> queueFamilyPropertiesList = m_PhysicalDevice.getQueueFamilyProperties();

    std::array<std::optional<uint32_t>, QueueType::Count> queueIndices {};

    for (uint32_t i = 0; i < queueFamilyPropertiesList.size(); ++i)
    {
        if (m_PhysicalDevice.getSurfaceSupportKHR(i, m_Surface))
        {
            queueIndices[QueueType::Present] = i;
        }

        if (queueFamilyPropertiesList[i].queueFlags | vk::QueueFlagBits::eGraphics)
        {
            queueIndices[QueueType::Graphics] = i;
        }

        if (queueFamilyPropertiesList[i].queueFlags | vk::QueueFlagBits::eTransfer)
        {
            queueIndices[QueueType::Transfer] = i;
        }

        if (std::ranges::all_of(queueIndices, [](const auto& opIndex) { return opIndex.has_value(); }))
        {
            break;
        }
    }

    for (size_t i = 0; i < queueIndices.size(); ++i)
    {
        m_QueueIndices[i] = queueIndices[i].value();
    }

    // Create queue only for each unique index
    const float queuePriority = 1.0f;
    const std::set<uint32_t> uniqueQueueIndices { m_QueueIndices.begin(), m_QueueIndices.end() };

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos {};

    for (const uint32_t uniqueQueueIndex : uniqueQueueIndices)
    {
        const vk::DeviceQueueCreateInfo queueCreateInfo { {}, uniqueQueueIndex, vk::ArrayProxyNoTemporaries(queuePriority) };

        queueCreateInfos.push_back(queueCreateInfo);
    }

    const std::vector<const char*> extensions {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    // todo: Assuming that we have device features
    vk::PhysicalDeviceFeatures physicalDeviceFeatures {};
    physicalDeviceFeatures.setSamplerAnisotropy(VK_TRUE);

    // No Layers
    // No Extensions
    const vk::DeviceCreateInfo deviceCreateInfo { {}, queueCreateInfos, {}, extensions, &physicalDeviceFeatures };

    m_Device = m_PhysicalDevice.createDevice(deviceCreateInfo);

    ASSERT(m_Device);

    for (uint32_t i = 0; i < QueueType::Count; ++i)
    {
        m_Queues[i] = m_Device.getQueue(m_QueueIndices[i], 0);
    }
}

void RendererVk::CreateSwapchain(HWND appWindow)
{
    m_SwapchainHandler = std::make_unique<SwapchainHandler>(m_PhysicalDevice, m_Device, m_Surface, appWindow);
}

void RendererVk::CreateHandlers()
{
    m_MemoryHandler = std::make_unique<MemoryHandler>(m_PhysicalDevice, m_Device);
}

void RendererVk::CreateRenderers()
{
    const vk::Extent2D extent { m_SwapchainHandler->GetExtent() };
    const RenderContext renderContext { m_Device, m_QueueIndices, m_Queues, MAX_FRAMES_IN_FLIGHT, m_SwapchainHandler->GetImageViews().size(), extent, extent, EDITOR };

#ifdef EDITOR
    // ImGui renderer needs to initialize first as we need to create image texture attachments from other render passes
    m_RenderImGui = std::make_unique<RenderImGui>(renderContext, m_Instance, m_PhysicalDevice);
#endif  // EDITOR

    m_RenderGBuffer = std::make_unique<RenderGBuffer>(renderContext);
    m_RenderFinal = std::make_unique<RenderFinal>(renderContext);
}

void RendererVk::CreateCommandPool()
{
    vk::CommandPoolCreateInfo createInfo { vk::CommandPoolCreateFlagBits::eResetCommandBuffer, m_QueueIndices[QueueType::Graphics] };

    createInfo.flags = vk::CommandPoolCreateFlagBits::eTransient;
    m_OneShotCommandPool = m_Device.createCommandPool(createInfo);
}

void RendererVk::CreateFramebuffers()
{
    ASSERT(m_SwapchainHandler != nullptr);

#ifdef EDITOR
    if (m_IsRenderToTarget)
    {
        m_SwapchainHandler->CreateFramebuffers(m_RenderImGui->GetRenderPass());
        m_RenderImGui->OnSwapchainFramebuffersChanged();
    }
    else
#endif  // EDITOR
    {
        m_SwapchainHandler->CreateFramebuffers(m_RenderFinal->GetRenderPass());
        m_RenderFinal->OnSwapchainFramebuffersChanged();
    }
}

void RendererVk::CreateSyncObjects()
{
    vk::SemaphoreCreateInfo semaphoreCreateInfo {};
    vk::FenceCreateInfo fenceCreateInfo { vk::FenceCreateFlagBits::eSignaled };

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_ImageAvailableSemaphores.push_back(m_Device.createSemaphore(semaphoreCreateInfo));
        m_InFlightFences.push_back(m_Device.createFence(fenceCreateInfo));
    }
}

vk::CommandBuffer RendererVk::CreateOneShotCommandBuffer()
{
    ASSERT(m_Device);

    const vk::CommandBufferAllocateInfo allocateInfo { m_OneShotCommandPool, {}, 1 };

    vk::CommandBuffer commandBuffer = m_Device.allocateCommandBuffers(allocateInfo).front();

    const vk::CommandBufferBeginInfo beginInfo { vk::CommandBufferUsageFlagBits::eOneTimeSubmit };

    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void RendererVk::SubmitOneShotCommandBuffer(const vk::CommandBuffer commandBuffer, const vk::QueueFlagBits targetQueue, const vk::Fence signalFence /*= {}*/)
{
    commandBuffer.end();

    const vk::SubmitInfo submitInfo { {}, {}, commandBuffer };
    const vk::Queue queue = targetQueue & vk::QueueFlagBits::eTransfer ? m_Queues[QueueType::Transfer] : m_Queues[QueueType::Graphics];

    queue.submit(submitInfo, signalFence);

    m_Device.waitIdle();
    m_Device.freeCommandBuffers(m_OneShotCommandPool, { commandBuffer });
}

VkDescriptorSet RendererVk::GetRenderAttachment() const
{
    return m_RenderFinal->GetRenderAttachment(m_CurrentFrame);
}
