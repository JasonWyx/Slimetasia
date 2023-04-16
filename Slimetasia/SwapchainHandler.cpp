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
    vk::FramebufferCreateInfo createInfo {
        .renderPass = renderPass,
        .width = m_Extent.width,
        .height = m_Extent.height,
        .layers = 1,
    };

    for (size_t i = 0; i < m_ImageViews.size(); ++i)
    {
        createInfo.setAttachments(m_ImageViews[i]);
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

    vk::SwapchainCreateInfoKHR swapchainCreateInfo {
        .surface = surface,
        .minImageCount = details.m_Capabilities.minImageCount + 1,
        .imageFormat = m_SurfaceFormat.format,
        .imageColorSpace = m_SurfaceFormat.colorSpace,
        .imageExtent = m_Extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0,
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
            .subresourceRange {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .levelCount = 1,
                .layerCount = 1,
            },
        };

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
    m_SurfaceFormat = {};
    m_PresentMode = {};
    m_Extent = {};
}
