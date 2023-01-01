#include "RendererVk.h"

#include <vulkan/vulkan_win32.h>

#include <cstring>
#include <iostream>
#include <optional>
#include <set>

#include "External Libraries/imgui/backends/imgui_impl_vulkan.h"

static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

RendererVk::RendererVk(HINSTANCE appInstance, HWND appWindow, const uint32_t windowWidth, const uint32_t windowHeight)
    : m_CurrentFrame { 0 }
{
    CreateInstance();
    CreateSurface(appInstance, appWindow);
    ChoosePhysicalDevice();
    CreateDevice();
    CreateSwapchain(appWindow);
    CreateDescriptorPool();
    CreateCommandPool();
    CreatePipeline();
    CreateRenderPass();
    CreateSyncObjects();

    InitializeImGui();
}

RendererVk::~RendererVk()
{
    ShutdownImGui();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_Device.destroyFence(m_InFlightFences[i]);
        m_Device.destroySemaphore(m_RenderFinishedSemaphores[i]);
        m_Device.destroySemaphore(m_ImageAvailableSemaphores[i]);
    }
    m_Device.freeCommandBuffers(m_CommandPool, m_CommandBuffers);
    m_Device.destroyCommandPool(m_CommandPool);
    m_Device.destroyRenderPass(m_RenderPass);
    m_Device.destroyDescriptorPool(m_DescriptorPool);
    m_SwapchainHandler.release();
    m_Device.destroy();
    m_Instance.destroy();
}

void RendererVk::Update(const float deltaTime)
{
    const vk::Result waitResult = m_Device.waitForFences(m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
    m_Device.resetFences(m_InFlightFences[m_CurrentFrame]);

    const uint32_t imageIndex = m_SwapchainHandler->AcquireNextImageIndex(m_ImageAvailableSemaphores[m_CurrentFrame]);

    DrawImGui();

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RendererVk::CreateInstance()
{
    // Instance layers
    const std::vector<const char*> layerNames
    {
#if defined(_DEBUG)
        "VK_LAYER_KHRONOS_validation"
#endif  // #if defined(_DEBUG)
        "VK_LAYER_KHRONOS_validation"
    };

    // Instance extensions
    const std::vector<const char*> extensionNames
    {
        VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#if defined(_DEBUG)
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME
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

    uint32_t queueIndex = 0;
    for (const vk::QueueFamilyProperties& queueFamilyProperties : queueFamilyPropertiesList)
    {
        if (m_PhysicalDevice.getSurfaceSupportKHR(queueIndex, m_Surface))
        {
            m_PresentQueueIndex = queueIndex;
        }

        if (queueFamilyProperties.queueFlags | vk::QueueFlagBits::eGraphics)
        {
            m_GraphicsQueueIndex = queueIndex;
        }

        if (m_PresentQueueIndex.has_value() && m_GraphicsQueueIndex.has_value())
        {
            break;
        }

        queueIndex++;
    }

    // Create queue only for each unique index
    const float queuePriority = 1.0f;
    const std::set<uint32_t> uniqueQueueIndices { m_PresentQueueIndex.value(), m_GraphicsQueueIndex.value() };

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

    m_PresentQueue = m_Device.getQueue(m_PresentQueueIndex.value(), 0);
    m_GraphicsQueue = m_Device.getQueue(m_GraphicsQueueIndex.value(), 0);
}

void RendererVk::CreateSwapchain(HWND appWindow)
{
    m_SwapchainHandler = std::make_unique<SwapchainHandler>(m_PhysicalDevice, m_Device, m_Surface, appWindow);
}

void RendererVk::CreateDescriptorPool()
{
    ASSERT(m_Device);

    std::vector<vk::DescriptorPoolSize> poolSizes {
        vk::DescriptorPoolSize {
            .type = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = 1000,  // MAX_FRAMES_IN_FLIGHT,
        },
        vk::DescriptorPoolSize {
            .type = vk::DescriptorType::eSampledImage,
            .descriptorCount = 1000,  // MAX_FRAMES_IN_FLIGHT,
        },
        vk::DescriptorPoolSize {
            .type = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = 1000,  // MAX_FRAMES_IN_FLIGHT,
        },
    };

    vk::DescriptorPoolCreateInfo createInfo {
        .maxSets = MAX_FRAMES_IN_FLIGHT,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };

    m_DescriptorPool = m_Device.createDescriptorPool(createInfo);
}

void RendererVk::CreateCommandPool()
{
    vk::CommandPoolCreateInfo createInfo {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = m_GraphicsQueueIndex.value(),
    };

    m_CommandPool = m_Device.createCommandPool(createInfo);

    // Create command buffer for imgui
    vk::CommandBufferAllocateInfo allocateInfo {
        .commandPool = m_CommandPool,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };

    m_ImGuiCommandBuffers = m_Device.allocateCommandBuffers(allocateInfo);
}

void RendererVk::CreatePipeline() {}

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

    const vk::RenderPassCreateInfo createInfo {
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };

    m_RenderPass = m_Device.createRenderPass(createInfo);
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
        .commandPool = m_CommandPool,
        .commandBufferCount = 1,
    };

    vk::CommandBuffer commandBuffer = m_Device.allocateCommandBuffers(allocateInfo).front();

    const vk::CommandBufferBeginInfo beginInfo {
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    };

    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void RendererVk::SubmitOneShotCommandBuffer(const vk::CommandBuffer commandBuffer)
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo {
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    m_GraphicsQueue.submit(submitInfo);
    m_Device.waitIdle();
    m_Device.freeCommandBuffers(m_CommandPool, { commandBuffer });
}

void RendererVk::InitializeImGui()
{
    ImGui::CreateContext();

    ImGui_ImplVulkan_InitInfo imguiInitInfo {
        .Instance = m_Instance,
        .PhysicalDevice = m_PhysicalDevice,
        .Device = m_Device,
        .QueueFamily = m_GraphicsQueueIndex.value_or(0),
        .Queue = m_GraphicsQueue,
        .DescriptorPool = m_DescriptorPool,
        .Subpass = 0,
        .MinImageCount = MAX_FRAMES_IN_FLIGHT,
        .ImageCount = MAX_FRAMES_IN_FLIGHT,
    };
    ImGui_ImplVulkan_Init(&imguiInitInfo, m_RenderPass);

    const vk::CommandBuffer createFontCommands = CreateOneShotCommandBuffer();
    ImGui_ImplVulkan_CreateFontsTexture(createFontCommands);
    SubmitOneShotCommandBuffer(createFontCommands);
}

void RendererVk::ShutdownImGui()
{
    ImGui_ImplVulkan_DestroyFontUploadObjects();
    ImGui_ImplVulkan_Shutdown();
}

void RendererVk::DrawImGui()
{
    ImDrawData* drawData = ImGui::GetDrawData();
    if (drawData == nullptr)
    {
        return;
    }

    ImGui_ImplVulkan_NewFrame();

    const vk::CommandBuffer commandBuffer = m_ImGuiCommandBuffers[m_CurrentFrame];
    const vk::CommandBufferBeginInfo beginInfo {};
    commandBuffer.reset();
    commandBuffer.begin(beginInfo);
    ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
    commandBuffer.end();

    const vk::SubmitInfo submitInfo {
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    m_GraphicsQueue.submit(submitInfo);
}
