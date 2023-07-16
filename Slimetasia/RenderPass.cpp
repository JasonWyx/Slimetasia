#ifndef USE_VULKAN

#include "RenderPass.h"

RenderPass::RenderPass(const iVector2& viewportSize)
    : m_ViewportSize(viewportSize)
{
}

RenderPass::~RenderPass() {}

void RenderPass::SetViewportSize(const iVector2& viewportSize)
{
    if (viewportSize != m_ViewportSize)
    {
        m_ViewportSize = viewportSize;
        BuildRenderTargets();
    }
}

#endif // !USE_VULKAN