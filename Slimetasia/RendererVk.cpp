#include "RendererVk.h"

#include <vulkan/vulkan_win32.h>

#include <cstring>
#include <iostream>
#include <optional>
#include <set>
#include <ranges>

#include "External Libraries/imgui/backends/imgui_impl_vulkan.h"
#include "External Libraries/imgui/backends/imgui_impl_win32.h"
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
    CreateDescriptorPool();
    CreateCommandPool();
    CreateRenderPass();
    CreateFramebuffers();
    CreateSyncObjects();

    InitializeImGui(appWindow);
}

RendererVk::~RendererVk()
{
    m_Device.waitIdle();

    m_RenderFinalComposition.release();

    ShutdownImGui();

    std::ranges::for_each(m_InFlightFences, [this](const vk::Fence& fence) { m_Device.destroyFence(fence); });
    std::ranges::for_each(m_RenderFinishedSemaphores, [this](const vk::Semaphore& semaphore) { m_Device.destroySemaphore(semaphore); });
    std::ranges::for_each(m_ImageAvailableSemaphores, [this](const vk::Semaphore& semaphore) { m_Device.destroySemaphore(semaphore); });
#ifdef EDITOR
    m_Device.freeCommandBuffers(m_ImGuiCommandPool, m_ImGuiCommandBuffers);
    m_Device.destroyCommandPool(m_ImGuiCommandPool);
#endif  // EDITOR
    m_Device.destroyCommandPool(m_OneShotCommandPool);
    m_Device.destroyRenderPass(m_RenderPass);
    m_Device.destroyDescriptorPool(m_DescriptorPool);
    m_SwapchainHandler.release();
    m_Device.destroy();
    m_Instance.destroy();
}

void RendererVk::InitializeRenderers()
{
    const RenderContext renderContext { m_Device, m_QueueIndices, m_Queues, MAX_FRAMES_IN_FLIGHT, m_SwapchainHandler->GetExtent() };

    m_RenderGBuffer = std::make_unique<RenderGBuffer>(renderContext);
    m_RenderFinalComposition = std::make_unique<RenderFinalComposition>(renderContext);
}

void RendererVk::Update(const float deltaTime)
{
    const vk::Result waitResult = m_Device.waitForFences(m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
    m_Device.resetFences(m_InFlightFences[m_CurrentFrame]);

    const vk::ResultValue<uint32_t>& swapchainAcquireResult = m_SwapchainHandler->AcquireNextImageIndex(m_ImageAvailableSemaphores[m_CurrentFrame]);

    if (swapchainAcquireResult.result == vk::Result::eErrorOutOfDateKHR)
    {
        m_SwapchainHandler->RecreateSwapchain(m_PhysicalDevice, m_Device, m_Surface, m_AppWindow);
        return;
    }

    const uint32_t imageIndex = swapchainAcquireResult.value;
    const std::vector<vk::Framebuffer>& framebuffers { m_SwapchainHandler->GetFramebuffers() };
    const std::vector<vk::Semaphore> waitSemaphores { m_ImageAvailableSemaphores[m_CurrentFrame] };
    const std::vector<vk::Semaphore> signalSemaphores { m_RenderFinishedSemaphores[m_CurrentFrame] };

#ifdef EDITOR
    const vk::CommandBuffer commandBuffer { m_ImGuiCommandBuffers[m_CurrentFrame] };
    commandBuffer.reset();

    const vk::CommandBufferBeginInfo beginInfo {};
    commandBuffer.begin(beginInfo);

    const vk::ClearValue clearColor {
        .color = { .float32 = {{ 0.0f, 0.0f, 0.0f, 1.0f }}, },
    };
    const vk::RenderPassBeginInfo renderPassBeginInfo {
        .renderPass = m_RenderPass,
        .framebuffer = framebuffers[imageIndex],
        .renderArea = { .extent = m_SwapchainHandler->GetExtent() },
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };

    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    DrawImGui(commandBuffer);
    commandBuffer.endRenderPass();

    commandBuffer.end();

    const vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    const vk::SubmitInfo commandSubmitInfo {
        .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = &waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .pSignalSemaphores = signalSemaphores.data(),
    };

    m_Queues[QueueType::Graphics].submit(commandSubmitInfo, m_InFlightFences[m_CurrentFrame]);
#endif  // EDITOR

    const vk::SwapchainKHR swapchain = m_SwapchainHandler->GetSwapchain();

    vk::PresentInfoKHR presentInfo {
        .waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .pWaitSemaphores = signalSemaphores.data(),
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &imageIndex,
    };

    const vk::Result presentResult = m_Queues[QueueType::Graphics].presentKHR(presentInfo);

    if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR)
    {
        m_SwapchainHandler->RecreateSwapchain(m_PhysicalDevice, m_Device, m_Surface, m_AppWindow);
    }

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RendererVk::OnWindowResize()
{
    // Wait idle is called internally
    m_SwapchainHandler->RecreateSwapchain(m_PhysicalDevice, m_Device, m_Surface, m_AppWindow);
    m_SwapchainHandler->CreateFramebuffers(m_RenderPass);
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
#if defined(_DEBUG)
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif  // #if defined(_DEBUG)
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

        if(std::ranges::all_of(queueIndices, [](const auto& opIndex) { return opIndex.has_value(); }))
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
    m_MemoryHandler = std::make_unique<MemoryHandler>(m_PhysicalDevice, m_Device);
}

void RendererVk::CreateDescriptorPool()
{
    ASSERT(m_Device);

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

    m_DescriptorPool = m_Device.createDescriptorPool(createInfo);
}

void RendererVk::CreateCommandPool()
{
    vk::CommandPoolCreateInfo createInfo {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = m_QueueIndices[QueueType::Graphics],
    };
#ifdef EDITOR
    m_ImGuiCommandPool = m_Device.createCommandPool(createInfo);

    vk::CommandBufferAllocateInfo allocateInfo {
        .commandPool = m_ImGuiCommandPool,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };
    m_ImGuiCommandBuffers = m_Device.allocateCommandBuffers(allocateInfo);
#endif  // EDITOR
    createInfo.flags = vk::CommandPoolCreateFlagBits::eTransient;
    m_OneShotCommandPool = m_Device.createCommandPool(createInfo);
}
void RendererVk::CreateRenderPass()
{
    ASSERT(static_cast<bool>(m_SwapchainHandler));
    ASSERT(m_Device);

    const vk::AttachmentDescription colorAttachment {
        .format = m_SwapchainHandler->GetSurfaceFormat().format,
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
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
    };

    const vk::RenderPassCreateInfo createInfo {
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    m_RenderPass = m_Device.createRenderPass(createInfo);
}

void RendererVk::CreateFramebuffers()
{
    ASSERT(m_RenderPass);
    ASSERT(static_cast<bool>(m_SwapchainHandler));

    m_SwapchainHandler->CreateFramebuffers(m_RenderPass);
}

void RendererVk::CreateSyncObjects()
{
    vk::SemaphoreCreateInfo semaphoreCreateInfo {};
    vk::FenceCreateInfo fenceCreateInfo {};

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_ImageAvailableSemaphores.push_back(m_Device.createSemaphore(semaphoreCreateInfo));
        m_RenderFinishedSemaphores.push_back(m_Device.createSemaphore(semaphoreCreateInfo));
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

void RendererVk::InitializeImGui(const HWND appWindow)
{
    ImGui_ImplVulkan_InitInfo imguiInitInfo {
        .Instance = m_Instance,
        .PhysicalDevice = m_PhysicalDevice,
        .Device = m_Device,
        .QueueFamily = m_QueueIndices[QueueType::Graphics],
        .Queue = m_Queues[QueueType::Graphics],
        .DescriptorPool = m_DescriptorPool,
        .Subpass = 0,
        .MinImageCount = MAX_FRAMES_IN_FLIGHT,
        .ImageCount = static_cast<uint32_t>(m_SwapchainHandler->GetImageViews().size()),
    };

    ImGui_ImplVulkan_Init(&imguiInitInfo, m_RenderPass);

    const vk::CommandBuffer createFontCommands = CreateOneShotCommandBuffer();
    ImGui_ImplVulkan_CreateFontsTexture(createFontCommands);
    SubmitOneShotCommandBuffer(createFontCommands, vk::QueueFlagBits::eTransfer);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void RendererVk::ShutdownImGui()
{
    ImGui_ImplVulkan_Shutdown();
}

void RendererVk::DrawImGui(const vk::CommandBuffer commandBuffer)
{
    ImDrawData* drawData = ImGui::GetDrawData();
    if (drawData == nullptr)
    {
        return;
    }

    ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
}
