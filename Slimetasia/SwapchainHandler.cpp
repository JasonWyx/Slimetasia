#include "SwapchainHandler.h"

#include <algorithm>
#include <numeric>
#include <ranges>

#include "Logger.h"
#include "RendererVk.h"

SwapchainHandler::SwapchainHandler(const vk::PhysicalDevice physicalDevice, const vk::Device device, const vk::SurfaceKHR surface, const HWND window)
    : DeviceObject { device }
{
    ASSERT(physicalDevice);
    ASSERT(device);
    ASSERT(surface);
    ASSERT(window);

    CreateSwapchain(physicalDevice, device, surface, window);
}

SwapchainHandler::~SwapchainHandler()
{
    DestroySwapchain();
}

void SwapchainHandler::CreateFramebuffers(const vk::RenderPass renderPass)
{
    for (size_t i = 0; i < m_ImageViews.size(); ++i)
    {
        const vk::FramebufferCreateInfo createInfo { {}, renderPass, m_ImageViews[i], m_Extent.width, m_Extent.height, 1 };

        m_Framebuffers.push_back(m_ContextDevice.createFramebuffer(createInfo));
    }
}

void SwapchainHandler::RecreateSwapchain(const vk::PhysicalDevice physicalDevice, const vk::Device device, const vk::SurfaceKHR surface, const HWND window)
{
    ASSERT(physicalDevice);
    ASSERT(device);
    ASSERT(surface);
    ASSERT(window);

    m_ContextDevice.waitIdle();

    DestroySwapchain();
    CreateSwapchain(physicalDevice, device, surface, window);
}

vk::ResultValue<uint32_t> SwapchainHandler::AcquireNextImageIndex(const vk::Semaphore imageAvailableSemaphore)
{
    return m_ContextDevice.acquireNextImageKHR(m_Swapchain, UINT64_MAX, imageAvailableSemaphore);
}

/*static*/ SwapchainHandler::SwapchainSupportDetails SwapchainHandler::QuerySwapchainSupportDetails(const vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR surface)
{
    SwapchainSupportDetails details { physicalDevice.getSurfaceCapabilitiesKHR(surface), physicalDevice.getSurfaceFormatsKHR(surface), physicalDevice.getSurfacePresentModesKHR(surface) };

    return details;
}

/*static*/ vk::SurfaceFormatKHR SwapchainHandler::ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats)
{
    for (const vk::SurfaceFormatKHR& surfaceFormat : formats)
    {
        if (surfaceFormat.format == vk::Format::eB8G8R8A8Unorm && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
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

void SwapchainHandler::CreateSwapchain(const vk::PhysicalDevice physicalDevice, const vk::Device device, const vk::SurfaceKHR surface, const HWND window)
{
    SwapchainSupportDetails details = QuerySwapchainSupportDetails(physicalDevice, surface);
    m_SurfaceFormat = ChooseSurfaceFormat(details.m_Formats);
    m_PresentMode = ChoosePresentMode(details.m_PresentModes);
    m_Extent = ChooseExtent(details.m_Capabilities, window);

    vk::SwapchainCreateInfoKHR swapchainCreateInfo { {},
                                                     surface,
                                                     details.m_Capabilities.minImageCount + 1,
                                                     m_SurfaceFormat.format,
                                                     m_SurfaceFormat.colorSpace,
                                                     m_Extent,
                                                     1,
                                                     vk::ImageUsageFlagBits::eColorAttachment,
                                                     vk::SharingMode::eExclusive,
                                                     {},
                                                     details.m_Capabilities.currentTransform,
                                                     vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                                     m_PresentMode,
                                                     VK_TRUE };

    m_Swapchain = device.createSwapchainKHR(swapchainCreateInfo);
    ASSERT(m_Swapchain);

    m_Images = device.getSwapchainImagesKHR(m_Swapchain);

    for (const vk::Image image : m_Images)
    {
        const vk::ImageViewCreateInfo imageViewCreateInfo { {}, image, vk::ImageViewType::e2D, m_SurfaceFormat.format, {}, vk::ImageSubresourceRange { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };

        m_ImageViews.push_back(device.createImageView(imageViewCreateInfo));
    }
}

void SwapchainHandler::DestroySwapchain()
{
    std::ranges::for_each(m_Framebuffers, [this](const vk::Framebuffer& framebuffer) { m_ContextDevice.destroyFramebuffer(framebuffer); });
    std::ranges::for_each(m_ImageViews, [this](const vk::ImageView& imageView) { m_ContextDevice.destroyImageView(imageView); });
    m_ContextDevice.destroySwapchainKHR(m_Swapchain);
    m_Framebuffers.clear();
    m_ImageViews.clear();
    m_Images.clear();
    m_SurfaceFormat = vk::SurfaceFormatKHR {};
    m_PresentMode = {};
    m_Extent = vk::Extent2D {};
}
