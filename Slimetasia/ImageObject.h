#pragma once

#include "DeviceObject.h"
#include "MemoryHandler.h"

class BufferObject;

class ImageObject : public DeviceObject
{
public:

    ImageObject(const vk::ImageCreateInfo& imageCreateInfo, const vk::MemoryPropertyFlags& memoryProperties);
    ~ImageObject();

    void GenerateView(const vk::ImageViewType& viewType, const vk::ImageAspectFlags& aspectFlags);
    void GenerateSampler(const vk::Filter filter, const vk::SamplerAddressMode addressMode);

    void TransitionImageLayout(const vk::ImageLayout layout);
    void CopyFrom(const BufferObject& source, const vk::DeviceSize& offset);

    vk::Image GetImage() const { return m_Image; }
    vk::ImageView GetView() const { return m_View; }
    MemoryInfo GetMemoryInfo() const { return m_MemoryInfo; }
    vk::Extent3D GetExtent() const { return m_Extent; }
    vk::Format GetFormat() const { return m_Format; }
    vk::ImageLayout GetLayout() const { return m_Layout; }
    vk::Sampler GetSampler() const { return m_Sampler; }

    static ImageObject* CreateImage(const vk::Format& format, const vk::Extent3D extent, const vk::ImageUsageFlags& usageFlags, const bool isHostVisible, const void* source);
    static ImageObject* CreateDepthImage(const vk::Format& format, const vk::Extent3D extent, const vk::ImageUsageFlags& usageFlags);

private:

    vk::Image m_Image;
    vk::ImageView m_View;
    MemoryInfo m_MemoryInfo;
    vk::Extent3D m_Extent;
    vk::Format m_Format;
    vk::ImageLayout m_Layout;
    vk::Sampler m_Sampler;
};