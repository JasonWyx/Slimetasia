#include "RendererVk.h"

#include <vulkan/vulkan_win32.h>

#include <cstring>
#include <iostream>
#include <optional>
#include <ranges>
#include <set>

#include "ShaderHelper.h"

static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

RendererVk::RendererVk(HINSTANCE appInstance, HWND appWindow, const uint32_t windowWidth, const uint32_t windowHeight)
    : m_AppWindow { appWindow }
    , m_CurrentFrame { 0 }
{
    CreateInstance();
    CreateSurface(appInstance, appWindow);
    ChoosePhysicalDevice();
    CreateDevice();
    CreateSwapchain(appWindow);
    CreateHandlers();
    CreateRenderers();
    CreateCommandPool();
    CreateFramebuffers();
    CreateSyncObjects();
}

RendererVk::~RendererVk()
{
    m_Device.waitIdle();

    // Release renderers
    m_RenderImGui.release();
    m_RenderFinalComposition.release();
    m_RenderGBuffer.release();

    // Release device objects
    std::ranges::for_each(m_ImageAvailableSemaphores, [this](const vk::Semaphore& semaphore) { m_Device.destroySemaphore(semaphore); });
    m_Device.destroyCommandPool(m_OneShotCommandPool);

    // Release handlers
    m_MemoryHandler.release();
    m_SwapchainHandler.release();

    // Release core objects
    m_Device.destroy();
    m_Instance.destroy();
}

void RendererVk::Update(const float deltaTime)
{
    const vk::Fence& inFlightFence = m_InFlightFences[m_CurrentFrame];
    const vk::Result waitResult = m_Device.waitForFences(inFlightFence, VK_TRUE, UINT64_MAX);
    m_Device.resetFences(inFlightFence);

    const vk::ResultValue<uint32_t>& swapchainAcquireResult = m_SwapchainHandler->AcquireNextImageIndex(m_ImageAvailableSemaphores[m_CurrentFrame]);

    if (swapchainAcquireResult.result == vk::Result::eErrorOutOfDateKHR)
    {
        OnWindowResize();
        return;
    }

    const uint32_t imageIndex = swapchainAcquireResult.value;
    const std::vector<vk::Framebuffer>& framebuffers { m_SwapchainHandler->GetFramebuffers() };
    const std::vector<vk::Semaphore> waitSemaphores { m_ImageAvailableSemaphores[m_CurrentFrame] };
    std::vector<vk::Semaphore> renderFinishedSemaphores {};

    const FrameInfo& frameInfo { m_CurrentFrame, imageIndex };

    // const RenderSyncObjects finalCompRenderSync = m_RenderFinalComposition->Render(frameInfo, waitSemaphores);
    // renderFinishedSemaphores.push_back(finalCompRenderSync.signaledSemaphore);

    const RenderSyncObjects imguiRenderSync = m_RenderImGui->Render(frameInfo, waitSemaphores, inFlightFence);
    renderFinishedSemaphores.push_back(imguiRenderSync.signaledSemaphore);

    const vk::SwapchainKHR swapchain = m_SwapchainHandler->GetSwapchain();

    vk::PresentInfoKHR presentInfo {
        .waitSemaphoreCount = static_cast<uint32_t>(renderFinishedSemaphores.size()),
        .pWaitSemaphores = renderFinishedSemaphores.data(),
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &imageIndex,
    };

    const vk::Result presentResult = m_Queues[QueueType::Present].presentKHR(presentInfo);

    if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR)
    {
        OnWindowResize();
    }

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RendererVk::OnWindowResize()
{
    // Wait idle is called internally
    m_SwapchainHandler->RecreateSwapchain(m_PhysicalDevice, m_Device, m_Surface, m_AppWindow);

    CreateFramebuffers();

    const vk::Extent2D& extent = m_SwapchainHandler->GetExtent();

    // m_RenderGBuffer->OnExtentChanged(extent);
    m_RenderFinalComposition->OnExtentChanged(extent);
    m_RenderImGui->OnExtentChanged(extent);
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
#endif // EDITOR
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

    const vk::ApplicationInfo applicationInfo {
        .pApplicationName = "Slimetasia",
        .applicationVersion = 1,
        .pEngineName = "Slimetasia Engine",
        .engineVersion = 1,
        .apiVersion = VK_API_VERSION_1_3,
    };

    const vk::InstanceCreateInfo createInfo {
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(layerNames.size()),
        .ppEnabledLayerNames = layerNames.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensionNames.size()),
        .ppEnabledExtensionNames = extensionNames.data(),
    };

    m_Instance = vk::createInstance(createInfo);
}

void RendererVk::CreateSurface(const HINSTANCE hInstance, const HWND hWindow)
{
    const vk::Win32SurfaceCreateInfoKHR createInfo {
        .hinstance = hInstance,
        .hwnd = hWindow,
    };

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
        const vk::DeviceQueueCreateInfo queueCreateInfo {
            .queueFamilyIndex = uniqueQueueIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        };

        queueCreateInfos.push_back(queueCreateInfo);
    }

    const std::vector<const char*> extensions {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    vk::DeviceCreateInfo createInfo {
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };
    // No Layers
    // No Extensions
    // No PhysicalDeviceFeatures

    m_Device = m_PhysicalDevice.createDevice(createInfo);

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
    const RenderContext renderContext { m_Device, m_QueueIndices, m_Queues, MAX_FRAMES_IN_FLIGHT, m_SwapchainHandler->GetExtent() };

    // m_RenderGBuffer = std::make_unique<RenderGBuffer>(renderContext);
    m_RenderFinalComposition = std::make_unique<RenderFinalComposition>(renderContext, m_SwapchainHandler);
    m_RenderImGui = std::make_unique<RenderImGui>(renderContext, m_Instance, m_PhysicalDevice, m_SwapchainHandler);
}

void RendererVk::CreateCommandPool()
{
    vk::CommandPoolCreateInfo createInfo {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = m_QueueIndices[QueueType::Graphics],
    };

    createInfo.flags = vk::CommandPoolCreateFlagBits::eTransient;
    m_OneShotCommandPool = m_Device.createCommandPool(createInfo);
}

void RendererVk::CreateFramebuffers()
{
    ASSERT(static_cast<bool>(m_SwapchainHandler));

    // m_SwapchainHandler->CreateFramebuffers(m_RenderFinalComposition->GetRenderPass());
    // m_RenderFinalComposition->InitializeAfterSwapchain();

#ifdef EDITOR
    m_SwapchainHandler->CreateFramebuffers(m_RenderImGui->GetRenderPass());
    m_RenderImGui->InitializeAfterSwapchain();
#endif  // EDITOR
}

void RendererVk::CreateSyncObjects()
{
    vk::SemaphoreCreateInfo semaphoreCreateInfo {};
    vk::FenceCreateInfo fenceCreateInfo { .flags = vk::FenceCreateFlagBits::eSignaled };

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_ImageAvailableSemaphores.push_back(m_Device.createSemaphore(semaphoreCreateInfo));
        m_InFlightFences.push_back(m_Device.createFence(fenceCreateInfo));
    }
}

vk::CommandBuffer RendererVk::CreateOneShotCommandBuffer()
{
    ASSERT(m_Device);

    const vk::CommandBufferAllocateInfo allocateInfo {
        .commandPool = m_OneShotCommandPool,
        .commandBufferCount = 1,
    };

    vk::CommandBuffer commandBuffer = m_Device.allocateCommandBuffers(allocateInfo).front();

    const vk::CommandBufferBeginInfo beginInfo {
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    };

    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void RendererVk::SubmitOneShotCommandBuffer(const vk::CommandBuffer commandBuffer, const vk::QueueFlagBits targetQueue, const vk::Fence signalFence /*= {}*/)
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo {
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    const vk::Queue queue = targetQueue & vk::QueueFlagBits::eTransfer ? m_Queues[QueueType::Transfer] : m_Queues[QueueType::Graphics];

    queue.submit(submitInfo, signalFence);

    m_Device.waitIdle();
    m_Device.freeCommandBuffers(m_OneShotCommandPool, { commandBuffer });
}
