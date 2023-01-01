#pragma once
#include <Windows.h>

#include <vulkan/vulkan.hpp>

#include "DeviceObject.h"

class SwapchainHandler : public DeviceObject
{
public:

    SwapchainHandler(const vk::PhysicalDevice physicalDevice, const vk::Device device, const vk::SurfaceKHR surface, const HWND window);
    ~SwapchainHandler();

    vk::SurfaceFormatKHR GetSurfaceFormat() const { return m_SurfaceFormat; }
    vk::Extent2D GetExtent() const { return m_Extent; }

    uint32_t AcquireNextImageIndex(const vk::Semaphore imageAvailableSemaphore);


private:

    struct SwapchainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR m_Capabilities;
        std::vector<vk::SurfaceFormatKHR> m_Formats;
        std::vector<vk::PresentModeKHR> m_PresentModes;
    };

    static SwapchainSupportDetails QuerySwapchainSupportDetails(const vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR surface);
    static vk::SurfaceFormatKHR ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats);
    static vk::PresentModeKHR ChoosePresentMode(const std::vector<vk::PresentModeKHR>& presentModes);
    static vk::Extent2D ChooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const HWND window);

    vk::SurfaceFormatKHR m_SurfaceFormat;
    vk::PresentModeKHR m_PresentMode;
    vk::Extent2D m_Extent;
    vk::SwapchainKHR m_Swapchain;
    std::vector<vk::Image> m_Images;
    std::vector<vk::ImageView> m_ImageViews;
};
