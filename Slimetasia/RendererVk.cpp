#include "RendererVk.h"

#include <vulkan/vulkan_win32.h>

#include <cstring>
#include <iostream>
#include <optional>
#include <set>

RendererVk::RendererVk(HINSTANCE hInstance, HWND hWindow, const uint32_t windowWidth, const uint32_t windowHeight)
{
    CreateInstance();
    CreateSurface(hInstance, hWindow);
    ChoosePhysicalDevice();
    CreateDevice();
}

RendererVk::~RendererVk()
{
    if (m_Instance)
    {
        m_Instance.destroy();
    }
}

void RendererVk::Update(const float deltaTime) {}

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
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,

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

    const vk::ApplicationInfo applicationInfo{
        .pApplicationName = "Slimetasia",
        .applicationVersion = 1,
        .pEngineName = "Slimetasia Engine",
        .engineVersion = 1,
        .apiVersion = VK_API_VERSION_1_3,
    };

    const vk::InstanceCreateInfo createInfo{
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
    const vk::Win32SurfaceCreateInfoKHR createInfo{
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

    std::optional<uint32_t> presentQueueIndex{};
    std::optional<uint32_t> graphicsQueueIndex{};

    uint32_t queueIndex = 0;
    for (const vk::QueueFamilyProperties& queueFamilyProperties : queueFamilyPropertiesList)
    {
        if (m_PhysicalDevice.getSurfaceSupportKHR(queueIndex, m_Surface))
        {
            presentQueueIndex = queueIndex;
        }

        if (queueFamilyProperties.queueFlags | vk::QueueFlagBits::eGraphics)
        {
            graphicsQueueIndex = queueIndex;
        }

        if (presentQueueIndex.has_value() && graphicsQueueIndex.has_value())
        {
            break;
        }

        queueIndex++;
    }

    // Create queue only for each unique index
    const float queuePriority = 1.0f;
    const std::set<uint32_t> uniqueQueueIndices{ presentQueueIndex.value(), graphicsQueueIndex.value() };

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};

    for (const uint32_t uniqueQueueIndex : uniqueQueueIndices)
    {
        const vk::DeviceQueueCreateInfo queueCreateInfo{
            .queueFamilyIndex = uniqueQueueIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        };

        queueCreateInfos.push_back(queueCreateInfo);
    }

    const std::vector<const char*> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    vk::DeviceCreateInfo createInfo{
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

    m_PresentQueue = m_Device.getQueue(presentQueueIndex.value(), 0);
    m_GraphicsQueue = m_Device.getQueue(graphicsQueueIndex.value(), 0);
}

void RendererVk::CreateSwapchain() {}
