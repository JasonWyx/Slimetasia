#include "SwapchainHandler.h"

#include <algorithm>
#include <numeric>

#include "Logger.h"
#include "RendererVk.h"

SwapchainHandler::SwapchainHandler(const vk::PhysicalDevice physicalDevice, const vk::Device device, const vk::SurfaceKHR surface, const HWND window)
    : DeviceObject { device }
{
    ASSERT(physicalDevice);
    ASSERT(device);
    ASSERT(surface);
    ASSERT(window);

    SwapchainSupportDetails details = QuerySwapchainSupportDetails(physicalDevice, surface);
    m_SurfaceFormat = ChooseSurfaceFormat(details.m_Formats);
    m_PresentMode = ChoosePresentMode(details.m_PresentModes);
    m_Extent = ChooseExtent(details.m_Capabilities, window);

    vk::SwapchainCreateInfoKHR swapchainCreateInfo {
        .surface = surface,
        .minImageCount = details.m_Capabilities.minImageCount + 1,
        .imageFormat = m_SurfaceFormat.format,
        .imageColorSpace = m_SurfaceFormat.colorSpace,
        .imageExtent = m_Extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .preTransform = details.m_Capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = m_PresentMode,
        .clipped = VK_TRUE,
    };

    m_Swapchain = device.createSwapchainKHR(swapchainCreateInfo);
    ASSERT(m_Swapchain);

    m_Images = device.getSwapchainImagesKHR(m_Swapchain);

    for (const vk::Image image : m_Images)
    {
        vk::ImageViewCreateInfo imageViewCreateInfo {
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = m_SurfaceFormat.format,
            .subresourceRange = { .levelCount = 1, .layerCount = 1 },
        };

        m_ImageViews.push_back(device.createImageView(imageViewCreateInfo));
    }
}

SwapchainHandler::~SwapchainHandler()
{
    for (const vk::ImageView imageView : m_ImageViews)
    {
        m_OwningDevice.destroyImageView(imageView);
    }
    m_OwningDevice.destroySwapchainKHR(m_Swapchain);
}

uint32_t SwapchainHandler::AcquireNextImageIndex(const vk::Semaphore imageAvailableSemaphore) 
{
    return m_OwningDevice.acquireNextImageKHR(m_Swapchain, UINT64_MAX, imageAvailableSemaphore).value;
}

/*static*/ SwapchainHandler::SwapchainSupportDetails SwapchainHandler::QuerySwapchainSupportDetails(const vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR surface)
{
    SwapchainSupportDetails details {
        .m_Capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface),
        .m_Formats = physicalDevice.getSurfaceFormatsKHR(surface),
        .m_PresentModes = physicalDevice.getSurfacePresentModesKHR(surface),
    };

    return details;
}

/*static*/ vk::SurfaceFormatKHR SwapchainHandler::ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats)
{
    for (const vk::SurfaceFormatKHR& surfaceFormat : formats)
    {
        if (surfaceFormat.format == vk::Format::eB8G8R8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return surfaceFormat;
        }
    }

    return formats.front();
}

/*static*/ vk::PresentModeKHR SwapchainHandler::ChoosePresentMode(const std::vector<vk::PresentModeKHR>& presentModes)
{
    for (const vk::PresentModeKHR& presentMode : presentModes)
    {
        // Buffered present mode (non-blocking)
        if (presentMode == vk::PresentModeKHR::eMailbox)
        {
            return presentMode;
        }
    }

    // Standard present mode (blocking)
    return vk::PresentModeKHR::eFifo;
}

/*static*/ vk::Extent2D SwapchainHandler::ChooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const HWND window)
{
    // Check if extent is valid
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    // Else use clamped to valid size
    else
    {
        RECT windowRect {};
        const bool result = GetWindowRect(window, &windowRect);
        ASSERT(result);

        const uint32_t width = windowRect.right - windowRect.left;
        const uint32_t height = windowRect.bottom - windowRect.top;

        vk::Extent2D extent { width, height };
        extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return extent;
    }
}
