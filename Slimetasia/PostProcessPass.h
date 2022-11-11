#pragma once
#include <GL/glew.h>

#include "Camera.h"
#include "CorePrerequisites.h"
#include "GeometryPass.h"

class PostProcessPass
{
public:

    PostProcessPass(iVector2 const& viewportSize);
    ~PostProcessPass();

    void Render(Camera& camera, GLuint renderTexture, GLuint mainFramebuffer);
    void SetViewportSize(iVector2 const& viewportSize);

private:

    HShader m_BasicDrawShader;
    HShader m_BloomPrePassShader;
    HShader m_GausBlurShader;

    iVector2 m_ViewportSize;
    GLuint m_Framebuffer;
    GLuint m_BloomPrePassTarget;
    GLuint m_BloomPostProcessTarget;

    GLuint m_VertexArray;
    GLuint m_VertexBuffer;

    void BuildRenderTargets();
};
