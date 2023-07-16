#pragma once
#ifndef USE_VULKAN

#include "MathDefs.h"

class Camera;
class RenderLayer;

class RenderPass
{
public:  // Functions

    RenderPass(const iVector2& viewportSize);
    virtual ~RenderPass();

    virtual void Render(Camera* camera, const RenderLayer& renderLayer) = 0;
    void SetViewportSize(const iVector2& viewportSize);

protected:  // Variables

    iVector2 m_ViewportSize;

protected:  // Functions

    virtual void BuildRenderTargets() = 0;
};

#endif // !USE_VULKAN