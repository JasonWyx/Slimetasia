#pragma once
#ifndef USE_VULKAN

#include <GL/glew.h>

#include "Camera.h"
#include "CorePrerequisites.h"

class FinalPass
{
public:

    FinalPass(iVector2 const& viewportSize);
    ~FinalPass();

    void Render(const Camera& camera, GLuint renderTexture);
    void SetViewportSize(iVector2 const& viewportSize);
    GLuint GetRenderTexture() const;
    GLuint GetFramebuffer() const;

private:

    HShader m_FinalPassShader;
    GLuint m_VertexArray;
    GLuint m_VertexBuffer;

    iVector2 m_ViewportSize;
    GLuint m_Framebuffer;
    GLuint m_RenderTarget;

    void BuildRenderTargets();
};

#endif // !USE_VULKAN